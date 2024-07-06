#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <locale>
#include <stdlib.h> // defines setenv
#include "utils/ErrorReporting.h"
#include "platform.h"  // For put_time and get_time vs strftime and strptime
#include "TimeUtils.h"

using TimeResolution = std::chrono::microseconds;

static std::locale g_locale;

clockValue specTimeGetTOD()
{
	auto currentTime = SClock::now();
	auto sinceEpoch = currentTime.time_since_epoch();
	int64_t microseconds = std::chrono::duration_cast<TimeResolution>(sinceEpoch).count();
	return (clockValue(microseconds));
}


PSpecString specTimeConvertToPrintable(int64_t sinceEpoch, std::string format)
{
	SClock::duration dur = std::chrono::microseconds(sinceEpoch);
	STimePoint tp(dur);
	auto tmc = SClock::to_time_t(tp);
	std::tm bt = *std::localtime(&tmc);
	unsigned int fractionalSecond = (unsigned int)(sinceEpoch % MICROSECONDS_PER_SECOND);
	std::ostringstream oss;
	unsigned char fractionalSecondLength = 0;

	if (format.length() >= 3) {
		auto l = format.length();
		if (format[l-3]=='%' && format[l-1]=='f' &&
				format[l-2]>='0' && format[l-2]<='6') {
			fractionalSecondLength = format[l-2] - '0';
			format = format.substr(0,l-3);
		}
	}

#ifdef PUT_TIME__SUPPORTED
	oss.imbue(g_locale);
	oss << std::put_time(&bt, format.c_str());
#else
	char timeFormatterString[256];
	strftime(timeFormatterString, 255, format.c_str(), &bt);
	oss << timeFormatterString;
#endif
	if (fractionalSecondLength) {
		oss << std::setw(fractionalSecondLength) << std::setfill('0');
		while (fractionalSecondLength < 6) {
			fractionalSecondLength++;
			fractionalSecond /= 10;
		}
		oss << fractionalSecond;
	}
	std::string ret = oss.str();
	return std::make_shared<std::string>(ret);
}

int64_t specTimeConvertFromPrintable(std::string printable, std::string format)
{
	// initialize t in case of missing fields
	std::time_t now = std::time(nullptr);
	std::tm t = *(std::localtime(&now));

	unsigned int fractionalSeconds = 0;
	unsigned char fractionalSecondLength = 0;
	std::string fractionalPart;

	if (format.length() >= 3) {
		auto l = format.length();
		if (format[l-3]=='%' && format[l-1]=='f' &&
				format[l-2]>='0' && format[l-2]<='6') {
			fractionalSecondLength = format[l-2] - '0';
			format = format.substr(0,l-3);
		}
	}

#ifdef PUT_TIME__SUPPORTED
	std::istringstream ss(printable);
	ss.imbue(g_locale);
	ss >> std::get_time(&t, format.c_str());
	if (ss.fail()) {
		return 0;
	}
	if (fractionalSecondLength > 0) {
		std::getline(ss, fractionalPart);
		if (fractionalPart.length() != fractionalSecondLength) {
			fractionalPart.resize(fractionalSecondLength, '0');
		}
	}
#else
	char* fractionalPartPtr = strptime(printable.c_str(), format.c_str(), &t);
	if (!fractionalPartPtr) {
		return 0;
	}
	if (fractionalSecondLength > 0) {
		fractionalPart = fractionalPartPtr;
		if (fractionalPart.length() != fractionalSecondLength) {
			fractionalPart.resize(fractionalSecondLength, '0');
		}
	}
#endif

	// automatic detection of DST - Issue #2
	t.tm_isdst = -1;
	std::time_t secondsSinceEpoch = std::mktime(&t);

	// take care of microseconds
	if (fractionalSecondLength > 0) {
		try {
			int extraZeros = 6 - int(fractionalPart.length());
			fractionalSeconds = std::stoi(fractionalPart);
			while (extraZeros > 0) {
				fractionalSeconds *= 10;
				extraZeros--;
			}
		} catch (std::invalid_argument& e) {
			fractionalSeconds = 0;
		}
	}

	uint64_t ret = uint64_t(secondsSinceEpoch) * MICROSECONDS_PER_SECOND + fractionalSeconds;

	return ret;
}

void specTimeSetTimeZone(const std::string& tzname)
{
	static const char sTZ[] = "TZ";
	setenv(sTZ, tzname.data(), 1);
}

void specTimeSetLocale(const std::string& _locale, bool throwIfInvalid)
{
	try {
		g_locale = std::locale(_locale);
	} catch(std::runtime_error& e) {
		std::string err = "Invalid locale <" + _locale + ">";
		if (throwIfInvalid) {
			MYTHROW(err);
		}
		std::cerr << err << std::endl;
	}
}

classifyingTimer::classifyingTimer()
{
	m_lastTimePoint = HClock::now();
	m_currentClass = timeClassInitializing;
	for (int i = timeClassInitializing ; i < timeClassLast ; i++) {
		m_nanoseconds[i] = 0;
	}
}

void classifyingTimer::changeClass(timeClasses _class)
{
	if (_class == m_currentClass) return;  
	MYASSERT(m_currentClass != timeClassLast);
	auto now = HClock::now();
	uint64_t duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now-m_lastTimePoint).count();
	m_nanoseconds[m_currentClass] += duration;
	m_currentClass = _class;
	m_lastTimePoint = now;
}

void classifyingTimer::dump(std::string title)
{
	static const std::string classTitles[] = {
			"Initializing",
			"Processing",
			"Waiting on IO",
			"Waiting on input queue",
			"Waiting on output queue",
			"Draining",
			"Last"
	};

	uint64_t totalDuration = 0;
	for (int i = timeClassInitializing ; i < timeClassLast ; i++) {
		totalDuration += m_nanoseconds[i];
	}

	std::ostringstream oss;
	oss.setf( std::ios::fixed, std:: ios::floatfield );
	oss << title << ":\n";
	for (int i = timeClassInitializing ; i < timeClassLast ; i++) {
		if (m_nanoseconds[i] > 0) {
			oss.precision(3);
			double percentage = double(100 * m_nanoseconds[i]) / double(totalDuration);
			oss << "\t" << classTitles[i] << ": ";
			if (m_nanoseconds[i] > NANOSECONDS_PER_SECOND) {
				oss << getSeconds(timeClasses(i)) << " seconds";
			} else if (m_nanoseconds[i] > NANOSECONDS_PER_MILLISECOND) {
				oss << getMilliSeconds(timeClasses(i)) << " ms";
			} else {
				oss << getMicroSeconds(timeClasses(i)) << " us";
			}

			oss << " (" << percentage << "%)\n";
		}
	}
	std::cerr << oss.str();
}

queueTimer::queueTimer()
{
	m_lastIncDec = m_lastTimePoint = HClock::now();
	m_elements = 0;
	m_ns_elems = 0;
	m_currentClass = queueTimeClassEmpty;
	for (int i = queueTimeClassEmpty ; i < queueTimeclassLast ; i++) {
		m_nanoseconds[i] = 0;
	}
}

void queueTimer::changeClass(queueTimeClasses _class, std::chrono::time_point<HClock> now)
{
	if (_class == m_currentClass) return;
	MYASSERT(m_currentClass != queueTimeclassLast);
	uint64_t duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now-m_lastTimePoint).count();
	m_nanoseconds[m_currentClass] += duration;
	m_currentClass = _class;
	m_lastTimePoint = now;
}

void queueTimer::dump(std::string title)
{
	uint64_t totalDuration = 0;
	for (int i = queueTimeClassEmpty ; i < queueTimeclassLast ; i++) {
		totalDuration += m_nanoseconds[i];
	}

	std::ostringstream oss;
	oss.setf( std::ios::fixed, std:: ios::floatfield );
	oss.precision(3);

	oss << title << ":\n";

	// empty
	double percentage = double(100 * m_nanoseconds[queueTimeClassEmpty]) / double(totalDuration);
	oss << "\tEmpty: " << getMilliSeconds(queueTimeClassEmpty) << " ms ("<<
			percentage << "%)\n";

	// full
	percentage = double(100 * m_nanoseconds[queueTimeClassFull]) / double(totalDuration);
	oss << "\tFull: " << getMilliSeconds(queueTimeClassFull) << " ms ("<<
			percentage << "%)\n";

	// average
	double averageFill = double(m_ns_elems) / double(totalDuration);
	oss << "\tAverage: " << averageFill << " (capacity = " << QUEUE_HIGH_WM << ")\n";

	std::cerr << oss.str();
}

void queueTimer::increment()
{
	auto now = HClock::now();
	if (m_elements == 1 && m_currentClass == queueTimeClassEmpty) {
		changeClass(queueTimeClassOther, now);
	} else if (m_elements == (QUEUE_HIGH_WM-1) && m_currentClass == queueTimeClassOther) {
		changeClass(queueTimeClassFull, now);
	}

	uint64_t duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now-m_lastIncDec).count();

	m_ns_elems += (duration * m_elements);
	m_elements++;
	m_lastIncDec = now;
}

void queueTimer::decrement()
{
	auto now = HClock::now();
	MYASSERT(m_elements > 0);
	if (m_elements == 1 && m_currentClass == queueTimeClassOther) {
		changeClass(queueTimeClassEmpty, now);
	} else if (m_elements == (QUEUE_HIGH_WM-1) && m_currentClass == queueTimeClassFull) {
		changeClass(queueTimeClassOther, now);
	}

	uint64_t duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now-m_lastIncDec).count();

	m_ns_elems += (duration * m_elements);
	m_elements--;
	m_lastIncDec = now;
}

void queueTimer::drain()
{
	changeClass(queueTimeclassLast, HClock::now());
}
