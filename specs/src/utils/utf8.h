#ifndef SPECS2016__UTILS__UTF8__H
#define SPECS2016__UTILS__UTF8__H

#include <string>

// Helpers for treating a byte string as a sequence of UTF-8 encoded Unicode
// code points.  A "character" here is a code point, not a grapheme cluster.
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

bool isAscii(const char* p, size_t len);
inline bool isAscii(const std::string& s) { return isAscii(s.data(), s.size()); }

// Is every byte part of a well-formed sequence?
bool isValid(const char* p, size_t len);
inline bool isValid(const std::string& s) { return isValid(s.data(), s.size()); }

size_t charCount(const char* p, size_t len);
inline size_t charCount(const std::string& s) { return charCount(s.data(), s.size()); }

// The byte offset of the 0-based character index charIdx.  Returns s.size()
// for charIdx == charCount(s), and std::string::npos if charIdx is greater.
size_t charToByte(const std::string& s, size_t charIdx);

// Substring by character offset and count, with std::string::substr semantics
// for an over-long count.  charStart beyond the end yields an empty string.
std::string substr(const std::string& s, size_t charStart, size_t count = std::string::npos);

// The Unicode White_Space property.
bool isSpace(char32_t cp);

// Decode a string into its code points.
std::u32string toCodePoints(const std::string& s);

// Encode a code point.  Values in the invalid-byte range produce the original
// byte, so that toCodePoints followed by encoding round-trips any input.
void append(std::string& s, char32_t cp);

// Is cp one of the code points in set?
inline bool contains(const std::u32string& set, char32_t cp)
{
	return std::u32string::npos != set.find(cp);
}

}  // namespace utf8

#endif
