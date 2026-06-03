#include <cstring>
#include <fstream>
#include "utils/ErrorReporting.h"
#include "Config.h"
#include "Reader.h"

uint64_t g_readRecordCounter = 0;
Reader* g_pReader = nullptr;
PSpecString g_pOOBSpecString = std::make_shared<std::string>();

void ReadAllRecordsIntoReaderQueue(Reader* r)
{
	r->startProcessing();
	while (!r->endOfSource()) {
		r->readIntoQueue();
	}
	r->startDraining();
}

Reader::~Reader()
{
	PSpecString ps = nullptr;
	End();
	while (!m_queue.empty()) {
		m_queue.wait_and_pop(ps);
	}
}

void Reader::selectStream(unsigned char idx)
{
	if (idx != DEFAULT_READER_IDX) {
		std::string err = "Attempted to select stream " + std::to_string(idx) + " when no secondary streams are defined.";
		MYTHROW(err);
	}
}

void Reader::End()
{
	if (mp_thread) {
		mp_thread->join();
	}

	mp_thread = nullptr;
}

PSpecString Reader::get(classifyingTimer& tmr, unsigned int& _readerCounter)
{
	PSpecString ret = nullptr;
	if (m_pUnreadString) {
		ret = m_pUnreadString;
		m_pUnreadString = nullptr;
		return ret;
	}
	if (eof()) {
		if (!m_bRanDry) {
			MYASSERT(_readerCounter>0);
			_readerCounter--;
		}
		m_bRanDry = true;
		return nullptr;
	}
	if (!g_bThreaded) {
		tmr.changeClass(timeClassIO);
		ret = getNextRecord();
		tmr.changeClass(timeClassProcessing);
		if (!ret) {
			MYASSERT(_readerCounter>0);
			_readerCounter--;
			m_bRanDry = true;
		}
		else {
			m_countRead++;
			m_countUsed++;
		}
		return ret;
	}
	tmr.changeClass(timeClassInputQueue);
	bool res = m_queue.wait_and_pop(ret);
	tmr.changeClass(timeClassProcessing);
	if (res) {
		m_countUsed++;
		return ret;
	} else {
		MYASSERT(_readerCounter>0);
		_readerCounter--;
		m_bRanDry = true;
		return nullptr;
	}
}

void Reader::pushBack(PSpecString ps)
{
	MYASSERT_WITH_MSG(m_pUnreadString==nullptr, "Only one record can be UNREAD at a time");
	m_pUnreadString = ps;
}

void Reader::readIntoQueue()
{
	if (!endOfSource()) {
		PSpecString nextRecord = getNextRecord();
		if (nextRecord) {
			m_Timer.changeClass(timeClassOutputQueue);
			m_queue.push(nextRecord);
			m_Timer.changeClass(timeClassProcessing);
			m_countRead++;
		} else {
			m_queue.Done();
		}
	}
}

void Reader::Begin() {
	if (g_bThreaded)
		mp_thread = std::unique_ptr<std::thread>(new std::thread(ReadAllRecordsIntoReaderQueue, this));
}

PSpecString Reader::peek(int offset)
{
	MYTHROW("Rolling context is not supported for this reader type");
	return nullptr;
}


StandardReader::StandardReader() {
	m_NeedToClose = false;
	m_EOF = false;
	m_buffer = nullptr;
	m_recfm = RECFM_DELIMITED;
	m_lineDelimiter = 0;
	m_forwardContextSize = 0;
	m_backwardContextSize = 0;
	m_currentRecord = nullptr;
	m_contextInitialized = false;
}

StandardReader::StandardReader(std::istream* f) {
	MYASSERT(f!=nullptr);
	m_EOF = false;
	if (!f->good()) {  // so it crashes if what we've been passed is not a stream pointer
		m_EOF = true;
	}
	m_File = std::shared_ptr<std::istream>(f);
	m_NeedToClose = false;
	m_buffer = nullptr;
	m_recfm = RECFM_DELIMITED;
	m_lineDelimiter = 0;
	m_forwardContextSize = 0;
	m_backwardContextSize = 0;
	m_currentRecord = nullptr;
	m_contextInitialized = false;
}

StandardReader::StandardReader(std::string& fn) {
	auto pInputFile = std::make_shared<std::ifstream>(fn);
	m_File = pInputFile;
	if (!pInputFile->is_open()) {
		std::string err = "File not found: " + fn;
		MYTHROW(err);
	}
	m_NeedToClose = true;
	m_EOF = false;
	m_buffer = nullptr;
	m_recfm = RECFM_DELIMITED;
	m_lineDelimiter = 0;
	m_forwardContextSize = 0;
	m_backwardContextSize = 0;
	m_currentRecord = nullptr;
	m_contextInitialized = false;
}

StandardReader::StandardReader(pipeType pipe) {
	m_pipe = pipe;
	m_NeedToClose = false;
	m_EOF = false;
	m_buffer = nullptr;
	m_recfm = RECFM_DELIMITED;
	m_lineDelimiter = 0;
	m_forwardContextSize = 0;
	m_backwardContextSize = 0;
	m_currentRecord = nullptr;
	m_contextInitialized = false;
}

StandardReader::~StandardReader() {
	if (m_NeedToClose) {
		auto pInputFile = std::dynamic_pointer_cast<std::ifstream>(m_File);
		if (pInputFile) pInputFile->close();
	}
	if (m_buffer) {
		free(m_buffer);
	}
}

void StandardReader::setFormatFixed(unsigned int lrecl, bool delimited)
{
	if (lrecl == 0) MYTHROW("lrecl must be greater than zero");
	m_recfm = delimited ? RECFM_FIXED_DELIMITED : RECFM_FIXED;
	m_lrecl = lrecl;
	if (!delimited) {
		m_buffer = (char*)malloc(lrecl);
		if (!m_buffer) {
			MYTHROW("Failed to allocate read buffer");
		}
	}
}

void StandardReader::setLineDelimiter(char c)
{
	m_lineDelimiter = c;
}

void StandardReader::setContextSizes(unsigned int forward, unsigned int backward)
{
	m_forwardContextSize = forward;
	m_backwardContextSize = backward;
}

PSpecString StandardReader::peek(int offset)
{
	if (offset == 0) {
		return m_currentRecord ? m_currentRecord : g_pOOBSpecString;
	}
	if (offset < 0) {
		unsigned int idx = (unsigned int)(-offset) - 1;
		if (idx >= m_backwardBuffer.size()) return g_pOOBSpecString;
		return m_backwardBuffer[m_backwardBuffer.size() - 1 - idx];
	}
	// offset > 0
	unsigned int idx = (unsigned int)offset - 1;
	if (idx >= m_forwardBuffer.size()) return g_pOOBSpecString;
	return m_forwardBuffer[idx];
}

bool StandardReader::endOfSource() {
	if (m_bAbort) return true;
	if (m_contextInitialized && !m_forwardBuffer.empty()) return false;
	return m_EOF;
}

PSpecString StandardReader::getNextRecord() {
	if (m_forwardContextSize == 0 && m_backwardContextSize == 0)
		return getNextRecordInternal();

	if (!m_contextInitialized) {
		m_currentRecord = getNextRecordInternal();
		if (!m_currentRecord) return nullptr;
		for (unsigned int i = 0; i < m_forwardContextSize; i++) {
			PSpecString rec = getNextRecordInternal();
			if (!rec) break;
			m_forwardBuffer.push_back(rec);
		}
		m_contextInitialized = true;
		return m_currentRecord;
	}

	// Shift window forward
	m_backwardBuffer.push_back(m_currentRecord);
	if (m_backwardBuffer.size() > m_backwardContextSize)
		m_backwardBuffer.pop_front();

	if (!m_forwardBuffer.empty()) {
		m_currentRecord = m_forwardBuffer.front();
		m_forwardBuffer.pop_front();
		PSpecString rec = getNextRecordInternal();
		if (rec) m_forwardBuffer.push_back(rec);
	} else {
		m_currentRecord = getNextRecordInternal();
	}

	return m_currentRecord;
}

PSpecString StandardReader::getNextRecordInternal() {
	std::string line;
	bool ok;
	switch (m_recfm) {
	case RECFM_FIXED_DELIMITED:
	case RECFM_DELIMITED: {
		if (0 != m_lineDelimiter) {
			m_Timer.changeClass(timeClassIO);
			if (m_NeedToClose) {
				ok = std::getline(*m_File, line, m_lineDelimiter) ? true : false;
			} else if (m_pipe) {
				int c = fgetc(m_pipe.get());
				ok = !feof(m_pipe.get());
				if (ok) {
					line = "";
					while (c!=EOF && c!=m_lineDelimiter) {
						line += c;
						c = fgetc(m_pipe.get());
					}
				}
			} else {
				ok = std::getline(std::cin, line, m_lineDelimiter) ? true : false;
			}
			m_Timer.changeClass(timeClassProcessing);
			if (!ok) {
				if (!m_EOF) g_readRecordCounter++;
				m_EOF = true;
				return nullptr;
			}
		} else {
			bool ok;
			m_Timer.changeClass(timeClassIO);
			if (m_NeedToClose) {
				ok = std::getline(*m_File, line) ? true : false;
			} else if (m_pipe) {
				int c = fgetc(m_pipe.get());
				ok = !feof(m_pipe.get());
				if (ok) {
					line = "";
					while (c!=EOF && c!='\n') {
						line += c;
						c = fgetc(m_pipe.get());
					}
				}
			} else {
				ok = std::getline(std::cin, line) ? true : false;
			}
			m_Timer.changeClass(timeClassProcessing);
			if (!ok) {
				if (!m_EOF) g_readRecordCounter++;
				m_EOF = true;
				return nullptr;
			}
		}
		// strip trailing newline if any
		if (!line.empty() && line.back() == '\n') {
			line.pop_back();
		}
		
		if (RECFM_FIXED_DELIMITED == m_recfm) {
			if (line.length() > m_lrecl) {
				line = line.substr(0,m_lrecl);
			} else {
				while (line.length() < m_lrecl) {
					line += " ";
				}
			}
		}
		g_readRecordCounter++;
		return std::make_shared<std::string>(line);
	}
	case RECFM_FIXED: {
		m_Timer.changeClass(timeClassIO);
		m_File->read(m_buffer, m_lrecl);
		m_Timer.changeClass(timeClassProcessing);
		if (m_File->gcount() < m_lrecl) {
			if (!m_EOF) g_readRecordCounter++;
			m_EOF = true;
			return nullptr;
		} else {
			g_readRecordCounter++;
			return std::make_shared<std::string>(m_buffer, m_lrecl);
		}
	}
	default:
		return nullptr;	
	}
}

TestReader::TestReader(size_t maxLineCount)
{
	mp_arr = (PSpecString*)calloc(maxLineCount, sizeof(PSpecString));
	m_count = m_idx = 0;
	m_MaxCount = maxLineCount;
}

TestReader::~TestReader()
{
	if (mp_arr) {
		// Need to alert the smart pointer that we're going to deallocate the whole array
		// Not so smart, is it?
		unsigned int i;
		for (i=0 ; i<m_count ; i++) {
			mp_arr[i] = nullptr;
		}
		free(mp_arr);
	}
}

void TestReader::InsertString(const char* s)
{
	if (m_count >= m_MaxCount) {
		MYTHROW("Attempting to insert too many lines into TestReader");
	}
	mp_arr[m_count++] = std::make_shared<std::string>(s);
}

void TestReader::InsertString(PSpecString ps)
{
	if (m_count >= m_MaxCount) {
		MYTHROW("Attempting to insert too many lines into TestReader");
	}
	mp_arr[m_count++] = ps;
}

PSpecString TestReader::peek(int offset)
{
	// m_idx points to the *next* record to read, so current record is m_idx-1
	int target = int(m_idx) - 1 + offset;
	if (target < 0 || target >= int(m_count)) {
		return g_pOOBSpecString;  // sentinel for out-of-bounds
	}
	return mp_arr[target];
}

// #include <cstring>  // for memset
// #include "utils/ErrorReporting.h"

#define ITERATE_VALID_STREAMS(i)                \
	unsigned char i;                            \
	for (i=0 ; i < maxReaderIdx+1 ; i++) {   \
		if (nullptr != readerArray[i]) {

#define ITERATE_VALID_STREAMS_END				\
		}                                       \
    }


multiReader::multiReader(PReader pDefaultReader)
{
	for (int i=0; i<MAX_INPUT_STREAMS; i++) {
		readerArray[i] = nullptr;
		stringArray[i] = nullptr;
	}
	readerIdx = DEFAULT_READER_IDX - 1;
	maxReaderIdx = readerIdx;
	readerArray[readerIdx] = pDefaultReader;
	bFirstGet = true;
	stopReaderIdx = STOP_STREAM_INVALID;
	readerCounter = 1;
}


multiReader::~multiReader()
{
	ITERATE_VALID_STREAMS(idx)
		readerArray[idx] = nullptr;
		readerCounter--;
	ITERATE_VALID_STREAMS_END
}

void multiReader::addStream(unsigned char idx, std::istream* f)
{
	MYASSERT_WITH_MSG(idx>0 && idx <= MAX_INPUT_STREAMS, "Invalid input stream number");
	idx--;  // Set to C-style index
	MYASSERT_WITH_MSG(nullptr==readerArray[idx], "Input stream is already defined");

	readerArray[idx] = std::make_shared<StandardReader>(f);
	if (idx > maxReaderIdx) maxReaderIdx = idx;
	readerCounter++;
}

void multiReader::addStream(unsigned char idx, std::string& fn)
{
	MYASSERT_WITH_MSG(idx>0 && idx <= MAX_INPUT_STREAMS, "Invalid input stream number");
	idx--;  // Set to C-style index
	MYASSERT_WITH_MSG(nullptr==readerArray[idx], "Input stream is already defined");

	readerArray[idx] = std::make_shared<StandardReader>(fn);
	if (idx > maxReaderIdx) maxReaderIdx = idx;
	readerCounter++;
}

void multiReader::selectStream(unsigned char idx, PSpecString* ppRecord)
{
	MYASSERT_WITH_MSG(idx>0 && idx <= MAX_INPUT_STREAMS, "Invalid input stream number");
	idx--;  // Set to C-style index
	MYASSERT_WITH_MSG(nullptr!=readerArray[idx], "Invalid input stream");

	if (readerIdx!=idx) {
		MYASSERT(nullptr == stringArray[readerIdx]);
		stringArray[readerIdx] = *ppRecord;
		*ppRecord = stringArray[idx];
		stringArray[idx] = nullptr;
		readerIdx = idx;
	}
}

void multiReader::Begin()
{
	ITERATE_VALID_STREAMS(idx)
		readerArray[idx]->Begin();
	ITERATE_VALID_STREAMS_END
	MYASSERT(stopReaderIdx != STOP_STREAM_INVALID);
}

PSpecString multiReader::get(classifyingTimer& tmr, unsigned int& _readerCounter)
{
	PSpecString ret = readerArray[readerIdx]->get(tmr, readerCounter);
	if (!ret) {
		/* The current stream ran dry. But is is time to stop? */
		if (STOP_STREAM_ANY==stopReaderIdx || readerIdx==(stopReaderIdx-1) || 0==readerCounter) {
			/* Yes, time to exit */
			_readerCounter--;
			return nullptr;
		}
		ret = std::make_shared<std::string>();
	}

	ITERATE_VALID_STREAMS(idx)
		if (stringArray[idx]) {
			MYASSERT(idx!=readerIdx);
			stringArray[idx] = readerArray[idx]->get(tmr, readerCounter);
			if (!stringArray[idx]) {
				if (STOP_STREAM_ANY==stopReaderIdx || idx==(stopReaderIdx-1) || 0==readerCounter) {
					_readerCounter--;
					return nullptr;
				}
				stringArray[idx] = std::make_shared<std::string>();
			}
		} else {
			MYASSERT(idx==readerIdx || bFirstGet);
			/* ret has already been read, and the stringArray slot remains NULL */
			if (bFirstGet && idx!=readerIdx) {
				stringArray[idx] = readerArray[idx]->get(tmr, readerCounter);
				if (!stringArray[idx]) {
					if (STOP_STREAM_ANY==stopReaderIdx || idx==(stopReaderIdx-1) || 0==readerCounter) {
						_readerCounter--;
						return nullptr;
					}
					stringArray[idx] = std::make_shared<std::string>();
				}
			}
		}
	ITERATE_VALID_STREAMS_END

	bFirstGet = false;
	return ret;
}

bool multiReader::endOfSource()
{
	MYTHROW("multiReader::endOfSource() should not have been called.");
	return false;
}

PSpecString multiReader::getNextRecord()
{
	MYTHROW("multiReader::getNextRecord() should not have been called.");
	return nullptr;
}
