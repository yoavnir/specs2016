#ifndef SPECS2016__PROCESSING__STRINGBUILDER__H
#define SPECS2016__PROCESSING__STRINGBUILDER__H

#include "utils/SpecString.h"
#include "ProcessingState.h"

class StringBuilder : public outputAgent {
public:
	StringBuilder();
	~StringBuilder();
	PSpecString    GetString();
	PSpecString    GetStringUnsafe();
	size_t         Length() { return mp_str ? mp_str->length() : 0; }
	// In UTF-8 mode, offset and all positions are in characters
	void           insert(PSpecString str, size_t offset);
	void           insertNext(PSpecString str);
	void           insertNextWord(PSpecString str);
	void           insertNextField(PSpecString str);
	void           setPadChar(const std::string& c) {m_pad = c;}
	void           setPadChar(char c)     {m_pad = std::string(1, c);}
	size_t         pos() override         {return m_pos;}
	// A copy, not mp_str itself: the caller may insert the result straight
	// back into this same builder (e.g. the OUTREC spec unit).
	PSpecString    outputRecord() override {
		return mp_str ? std::make_shared<std::string>(*mp_str) : std::make_shared<std::string>();
	}
	size_t         getPosition() const    {return m_pos;}
	void           setPosition(size_t p)  {m_pos = p;}
	void           setString(PSpecString s) {mp_str = s; m_strAscii = -1;}
	void           clear()                {mp_str = nullptr; m_pos = 1; m_strAscii = -1;}
	PSpecString    PeekString() const     {return mp_str;}
private:
	bool            strIsAscii();
	PSpecString     mp_str;
	size_t          m_pos; // for Next, NextWord, NextField
	std::string     m_pad;
	int             m_strAscii;  // is mp_str all ASCII?  -1 = not yet known
};

#endif
