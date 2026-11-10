#include <cstdint>
#include <cstring>
#include <algorithm>
#define UTF8PROC_STATIC
#include "utils/utf8proc/utf8proc.h"
#include "utils/ErrorReporting.h"
#include "utf8.h"

namespace utf8 {

static inline bool isCont(unsigned char c)
{
	return 0x80 == (c & 0xC0);
}

// Returns the length of a well-formed sequence starting at p, or 0 if the
// bytes at p do not form one.
static size_t wellFormedLen(const char* p, size_t avail)
{
	const unsigned char* u = (const unsigned char*)p;
	unsigned char b0 = u[0];
	if (b0 < 0x80) return 1;

	size_t len;
	unsigned char lo = 0x80, hi = 0xBF;   // valid range for the second byte
	if (b0 >= 0xC2 && b0 <= 0xDF) {
		len = 2;
	} else if (b0 >= 0xE0 && b0 <= 0xEF) {
		len = 3;
		if (b0 == 0xE0) lo = 0xA0;        // overlong
		if (b0 == 0xED) hi = 0x9F;        // surrogates
	} else if (b0 >= 0xF0 && b0 <= 0xF4) {
		len = 4;
		if (b0 == 0xF0) lo = 0x90;        // overlong
		if (b0 == 0xF4) hi = 0x8F;        // above U+10FFFF
	} else {
		return 0;
	}

	if (avail < len) return 0;
	if (u[1] < lo || u[1] > hi) return 0;
	for (size_t i = 2; i < len; i++) {
		if (!isCont(u[i])) return 0;
	}
	return len;
}

size_t seqLen(const char* p, size_t avail)
{
	size_t len = wellFormedLen(p, avail);
	return len ? len : 1;
}

char32_t decode(const char* p, size_t avail, size_t& len)
{
	const unsigned char* u = (const unsigned char*)p;
	len = wellFormedLen(p, avail);
	switch (len) {
	case 1:
		return u[0];
	case 2:
		return (char32_t(u[0] & 0x1F) << 6) | (u[1] & 0x3F);
	case 3:
		return (char32_t(u[0] & 0x0F) << 12) | (char32_t(u[1] & 0x3F) << 6) | (u[2] & 0x3F);
	case 4:
		return (char32_t(u[0] & 0x07) << 18) | (char32_t(u[1] & 0x3F) << 12) |
				(char32_t(u[2] & 0x3F) << 6) | (u[3] & 0x3F);
	default:
		len = 1;
		return INVALID_BYTE_BASE + u[0];
	}
}

bool isAscii(const std::string& s)
{
	return !std::any_of(s.begin(), s.end(),
        [](unsigned char c) { return c & 0x80; });
}

bool isValid(const char* p, size_t len)
{
	size_t i = 0;
	while (i < len) {
		size_t l = wellFormedLen(p + i, len - i);
		if (!l) return false;
		i += l;
	}
	return true;
}

static size_t nextGraphemeEnd(const char* p, size_t len, size_t start)
{
	size_t firstLen;
	char32_t previous = decode(p + start, len - start, firstLen);
	size_t i = start + firstLen;
	utf8proc_int32_t state = 0;
	while (i < len) {
		size_t nextLen;
		char32_t next = decode(p + i, len - i, nextLen);
		bool invalid = previous >= INVALID_BYTE_BASE && previous <= INVALID_BYTE_BASE + 0xFF;
		invalid = invalid || (next >= INVALID_BYTE_BASE && next <= INVALID_BYTE_BASE + 0xFF);
		if (invalid || utf8proc_grapheme_break_stateful(previous, next, &state)) break;
		previous = next;
		i += nextLen;
	}
	return i;
}

size_t charCount(const char* p, size_t len)
{
	size_t ret = 0;
	size_t i = 0;
	while (i < len) {
		i = nextGraphemeEnd(p, len, i);
		ret++;
	}
	return ret;
}

size_t charToByte(const std::string& s, size_t charIdx)
{
	const char* p = s.data();
	size_t len = s.size();
	size_t i = 0;
	while (charIdx > 0 && i < len) {
		i = nextGraphemeEnd(p, len, i);
		charIdx--;
	}
	return (charIdx == 0) ? i : std::string::npos;
}

std::string substr(const std::string& s, size_t charStart, size_t count)
{
	size_t bStart = charToByte(s, charStart);
	if (bStart == std::string::npos) return std::string();
	if (count == std::string::npos) return s.substr(bStart);

	size_t len = s.size();
	size_t bEnd = bStart;
	while (count > 0 && bEnd < len) {
		bEnd = nextGraphemeEnd(s.data(), len, bEnd);
		count--;
	}
	return s.substr(bStart, bEnd - bStart);
}

bool isSpace(char32_t cp)
{
	if (cp < 0x80) {
		return cp == 0x20 || (cp >= 0x09 && cp <= 0x0D);
	}
	switch (cp) {
	case 0x0085:
	case 0x00A0:
	case 0x1680:
	case 0x2028:
	case 0x2029:
	case 0x202F:
	case 0x205F:
	case 0x3000:
		return true;
	default:
		return (cp >= 0x2000 && cp <= 0x200A);
	}
}

std::u32string toCodePoints(const std::string& s)
{
	std::u32string ret;
	const char* p = s.data();
	size_t len = s.size();
	size_t i = 0;
	while (i < len) {
		size_t l;
		ret.push_back(decode(p + i, len - i, l));
		i += l;
	}
	return ret;
}

void append(std::string& s, char32_t cp)
{
	if (cp < 0x80) {
		s += char(cp);
	} else if (cp >= INVALID_BYTE_BASE + 0x80 && cp <= INVALID_BYTE_BASE + 0xFF) {
		s += char(cp - INVALID_BYTE_BASE);
	} else if (cp < 0x800) {
		s += char(0xC0 | (cp >> 6));
		s += char(0x80 | (cp & 0x3F));
	} else if (cp < 0x10000) {
		s += char(0xE0 | (cp >> 12));
		s += char(0x80 | ((cp >> 6) & 0x3F));
		s += char(0x80 | (cp & 0x3F));
	} else {
		s += char(0xF0 | (cp >> 18));
		s += char(0x80 | ((cp >> 12) & 0x3F));
		s += char(0x80 | ((cp >> 6) & 0x3F));
		s += char(0x80 | (cp & 0x3F));
	}
}

std::string fromCodePoints(const std::u32string& cps)
{
	std::string ret;
	ret.reserve(cps.size());
	for (char32_t cp : cps) append(ret, cp);
	return ret;
}

GraphemeString::GraphemeString(size_t count, char c)
	: m_chars(count, std::string(1, c))
{
}

GraphemeString::GraphemeString(std::vector<std::string> chars)
	: m_chars(std::move(chars))
{
}

GraphemeString GraphemeString::substr(size_t pos, size_t count) const
{
	if (pos > m_chars.size()) return GraphemeString();
	size_t end = count == npos || count > m_chars.size() - pos
			? m_chars.size() : pos + count;
	return GraphemeString(std::vector<std::string>(m_chars.begin() + pos, m_chars.begin() + end));
}

std::string canonicalGrapheme(const std::string& g)
{
	if (isAscii(g)) return g;
	utf8proc_uint8_t* p = utf8proc_NFC(reinterpret_cast<const utf8proc_uint8_t*>(g.c_str()));
	if (!p) return g;
	std::string ret(reinterpret_cast<char*>(p));
	free(p);
	return ret;
}

static std::vector<std::string> canonicalGraphemes(const std::vector<std::string>& v)
{
	std::vector<std::string> ret;
	ret.reserve(v.size());
	for (const std::string& g : v) ret.push_back(canonicalGrapheme(g));
	return ret;
}

size_t GraphemeString::find(const GraphemeString& needle, size_t pos) const
{
	if (pos > m_chars.size()) return npos;
	if (needle.empty()) return pos;
	std::vector<std::string> h = canonicalGraphemes(m_chars), n = canonicalGraphemes(needle.m_chars);
	auto it = std::search(h.begin() + pos, h.end(), n.begin(), n.end());
	return it == h.end() ? npos : size_t(it - h.begin());
}

size_t GraphemeString::rfind(const GraphemeString& needle) const
{
	if (needle.empty()) return m_chars.size();
	std::vector<std::string> h = canonicalGraphemes(m_chars), n = canonicalGraphemes(needle.m_chars);
	auto it = std::find_end(h.begin(), h.end(), n.begin(), n.end());
	return it == h.end() ? npos : size_t(it - h.begin());
}

size_t GraphemeString::find(const value_type& ch) const
{
	std::string c = canonicalGrapheme(ch);
	auto it = std::find_if(m_chars.begin(), m_chars.end(), [&](const std::string& g) { return canonicalGrapheme(g) == c; });
	return it == m_chars.end() ? npos : size_t(it - m_chars.begin());
}

GraphemeString& GraphemeString::operator+=(const GraphemeString& other)
{
	m_chars.insert(m_chars.end(), other.m_chars.begin(), other.m_chars.end());
	return *this;
}

GraphemeString& GraphemeString::operator+=(const value_type& ch)
{
	m_chars.push_back(ch);
	return *this;
}

GraphemeString toGraphemes(const std::string& s)
{
	std::vector<std::string> ret;
	size_t start = 0;
	while (start < s.size()) {
		size_t end = nextGraphemeEnd(s.data(), s.size(), start);
		ret.push_back(s.substr(start, end - start));
		start = end;
	}
	return GraphemeString(std::move(ret));
}

std::string fromGraphemes(const GraphemeString& s)
{
	std::string ret;
	for (const std::string& ch : s.data()) ret += ch;
	return ret;
}


std::string make_printable(const std::string& s)
{
	auto gs = toGraphemes(s);
	GraphemeString gt;
	bool bLastGraphemeUnprintable = false;
	for (auto& g : gs) {
		if (bLastGraphemeUnprintable) {
			gt += ".";
			bLastGraphemeUnprintable = false;
		}
		if (1==g.length() && g[0]<32) {
			bLastGraphemeUnprintable = true;  // Will print a dot on the next char, but not if this is the last
		} else gt += g;
	}
	return fromGraphemes(gt);
}

}  // namespace utf8
