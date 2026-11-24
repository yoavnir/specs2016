#ifndef SPECS2016__UTILS__UTF8__H
#define SPECS2016__UTILS__UTF8__H

#include <string>
#include <vector>
#include <algorithm>

extern bool g_bSupportUTF8;

// Helpers for treating a byte string as a sequence of UTF-8 encoded Unicode
// grapheme clusters. A "character" here is an extended grapheme cluster.
//
// Decoding is lenient: a byte that does not start a well-formed sequence
// (stray continuation byte, truncated or overlong sequence, encoded surrogate,
// value above U+10FFFF) counts as a single one-byte character.  Such a byte
// decodes to U+DC80..U+DCFF (the Python "surrogateescape" convention), so it
// never compares equal to a real character.  None of these functions read
// past the given length.

namespace utf8 {

static const char32_t INVALID_BYTE_BASE = 0xDC00;

// Length in bytes (1-4) of the character starting at p.  avail must be > 0.
size_t seqLen(const char* p, size_t avail);

// Decode the character starting at p; its length in bytes goes into len.
char32_t decode(const char* p, size_t avail, size_t& len);

bool isAscii(const std::string& s);

// Is every byte part of a well-formed sequence?
bool isValid(const char* p, size_t len);
inline bool isValid(const std::string& s) { return isValid(s.data(), s.size()); }

size_t charCount(const char* p, size_t len);
inline size_t charCount(const std::string& s) { return charCount(s.data(), s.size()); }

// The byte offset of the 0-based character index charIdx.  Returns s.size()
// for charIdx == charCount(s), and std::string::npos if charIdx is greater.
size_t charToByte(const std::string& s, size_t charIdx);

// The byte offset of the end of the 0-based n-th *complete* grapheme cluster,
// or std::string::npos if fewer than n complete clusters are available.
// A trailing truncated multi-byte sequence (a lead byte with too few
// continuation bytes) is not counted as a complete cluster, so callers can
// keep reading until a record boundary is well-formed.
// bFinal is false unless the caller has consumed the whole input and p,len
// point to the rest of the available bytes. When bFinal is false, reaching
// the end of the len bytes means the n-th cluster may still be extended by a
// combining mark not yet read, so the function returns npos.
size_t nthGraphemeEnd(const char* p, size_t len, size_t n, bool bFinal);
inline size_t nthGraphemeEnd(const std::string& s, size_t n, bool bFinal) { return nthGraphemeEnd(s.data(), s.size(), n, bFinal); }

// Substring by character offset and count, with std::string::substr semantics
// for an over-long count.  charStart beyond the end yields an empty string.
std::string substr(const std::string& s, size_t charStart, size_t count = std::string::npos);

// The Unicode White_Space property.
bool isSpace(char32_t cp);

// The number of terminal columns taken by one grapheme cluster: 0 for control characters and combining marks, 2 for
// wide (East Asian) characters and emoji, and 1 otherwise. A cluster takes the width of its widest code point.
// An invalid byte takes one column.
int graphemeWidth(const std::string& g);

// The length in bytes of the longest prefix of p that is an identifier: a letter or underscore, followed by letters,
// digits and underscores. If unicode is true, a letter is any Unicode letter, and a digit is any Unicode decimal digit
// or number letter; a combining mark may also follow the first character.
size_t identifierLength(const char* p, size_t len, bool unicode);

// Decode a string into its code points.
std::u32string toCodePoints(const std::string& s);

// Encode a code point.  Values in the invalid-byte range produce the original
// byte, so that toCodePoints followed by encoding round-trips any input.
void append(std::string& s, char32_t cp);

// Encode a sequence of code points; the inverse of toCodePoints.
std::string fromCodePoints(const std::u32string& cps);

// A string split into extended grapheme clusters. It supplies the subset of
// std::string's interface used by the ALU's byte/grapheme-generic algorithms.
class GraphemeString {
public:
	using value_type = std::string;
	static const size_t npos = std::string::npos;

	GraphemeString() = default;
	GraphemeString(size_t count, char c);
	explicit GraphemeString(std::vector<std::string> chars);

	size_t length() const { return m_chars.size(); }
	size_t size() const { return m_chars.size(); }
	bool empty() const { return m_chars.empty(); }
	void reverse() { std::reverse(m_chars.begin(), m_chars.end()); }
	const value_type& operator[](size_t i) const { return m_chars[i]; }
	GraphemeString substr(size_t pos, size_t count = npos) const;
	size_t find(const GraphemeString& needle, size_t pos = 0) const;
	size_t rfind(const GraphemeString& needle) const;
	size_t find(const value_type& ch) const;
	GraphemeString& operator+=(const GraphemeString& other);
	GraphemeString& operator+=(const value_type& ch);
	friend GraphemeString operator+(GraphemeString lhs, const GraphemeString& rhs)
	{
		return lhs += rhs;
	}
	const std::vector<std::string>& data() const { return m_chars; }
	using const_iterator = std::vector<std::string>::const_iterator;
	const_iterator begin() const { return m_chars.begin(); }
	const_iterator end() const { return m_chars.end(); }
	const_iterator cbegin() const { return m_chars.cbegin(); }
	const_iterator cend() const { return m_chars.cend(); }

private:
	std::vector<std::string> m_chars;
};

// The NFC form of a single grapheme, so that equivalent encodings compare equal.
std::string canonicalGrapheme(const std::string& g);

// The string with every grapheme cluster in canonical (NFC) form. Invalid bytes are left as they are.
std::string canonicalString(const std::string& s);

GraphemeString toGraphemes(const std::string& s);
std::string fromGraphemes(const GraphemeString& s);

std::string make_printable(const std::string& s);

// Is cp one of the code points in set?
inline bool contains(const std::u32string& set, char32_t cp)
{
	return std::u32string::npos != set.find(cp);
}

}  // namespace utf8

#define UTF8_STRING_IS_ASCII(s)           ((!g_bSupportUTF8) || utf8::isAscii((s)))
#define UTF8_STRING_NOT_ASCII(s)          (g_bSupportUTF8 && (false==utf8::isAscii((s))))
#define UTF8_STRINGS_NOT_ASCII2(s1,s2)    (g_bSupportUTF8 && ((false==utf8::isAscii((s1))) || (false==utf8::isAscii((s2)))))
#define UTF8_STRINGS_NOT_ASCII3(s1,s2,s3) (g_bSupportUTF8 && ((false==utf8::isAscii((s1))) || (false==utf8::isAscii((s2))) || (false==utf8::isAscii((s3)))))

#endif
