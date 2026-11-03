#include <cstdint>
#include <cstring>
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

bool isAscii(const char* p, size_t len)
{
	// Eight bytes at a time; memcpy keeps this free of alignment and aliasing issues
	static const uint64_t highBits = 0x8080808080808080ULL;
	size_t i = 0;
	for ( ; i + 8 <= len; i += 8) {
		uint64_t w;
		memcpy(&w, p + i, 8);
		if (w & highBits) return false;
	}
	for ( ; i < len; i++) {
		if (p[i] & 0x80) return false;
	}
	return true;
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

size_t charCount(const char* p, size_t len)
{
	size_t ret = 0;
	size_t i = 0;
	while (i < len) {
		i += (p[i] & 0x80) ? seqLen(p + i, len - i) : 1;
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
		i += (p[i] & 0x80) ? seqLen(p + i, len - i) : 1;
		charIdx--;
	}
	return (charIdx == 0) ? i : std::string::npos;
}

std::string substr(const std::string& s, size_t charStart, size_t count)
{
	size_t bStart = charToByte(s, charStart);
	if (bStart == std::string::npos) return std::string();
	if (count == std::string::npos) return s.substr(bStart);

	const char* p = s.data();
	size_t len = s.size();
	size_t bEnd = bStart;
	while (count > 0 && bEnd < len) {
		bEnd += (p[bEnd] & 0x80) ? seqLen(p + bEnd, len - bEnd) : 1;
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

}  // namespace utf8
