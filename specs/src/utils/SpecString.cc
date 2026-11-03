#include <cstring>  // for memcpy
#include "processing/Config.h"
#include "ErrorReporting.h"
#include "utf8.h"
#include "SpecString.h"


std::string SpecString_Pad(const std::string& pad, size_t count)
{
	if (pad.size() == 1) return std::string(count, pad[0]);
	std::string ret;
	ret.reserve(count * pad.size());
	while (count-- > 0) ret += pad;
	return ret;
}

static void computeEllipsis(size_t newSize, ellipsisSpec es, size_t& prefixLength, size_t& suffixLength)
{
	size_t totalLength = newSize - 3;

	switch (es) {
	case ellipsisSpecLeft:
		prefixLength = 0;
		suffixLength = totalLength;
		break;
	case ellipsisSpecThird:
		prefixLength = totalLength / 3;
		suffixLength = totalLength - prefixLength;
		break;
	case ellipsisSpecHalf:
		prefixLength = totalLength / 2;
		suffixLength = totalLength - prefixLength;
		break;
	case ellipsisSpecTwoThirds:
		suffixLength = totalLength / 3;
		prefixLength = totalLength - suffixLength;
		break;
	case ellipsisSpecRight:
		prefixLength = totalLength;
		suffixLength = 0;
		break;
	default:
		MYTHROW("Internal Error");
	}
}

static void SpecString_ResizeBytes(PSpecString ps, size_t newSize, char padChar, outputAlignment oa, ellipsisSpec es)
{
	if ((ellipsisSpecNone != es) && (newSize < ps->size()) && (newSize >= 3)) {
		size_t prefixLength, suffixLength;
		computeEllipsis(newSize, es, prefixLength, suffixLength);
		*ps = ps->substr(0,prefixLength) + "..." + ps->substr(ps->length() - suffixLength);
	}

	if (oa!=outputAlignmentLeft) {
		int diffSize = ((int)newSize - (int)ps->size());
		if (diffSize == 0) return;
		if (diffSize < 0) {
			if (oa==outputAlignmentRight) {
				// Cut off enough bytes from the start
				*ps = ps->substr(size_t(-diffSize));
				return;
			} else { // oa==outputAlignmentCenter
				// Cut off just a half
				*ps = ps->substr(size_t(-diffSize/2));
			}
		} else { // diffSize > 0
			if (oa==outputAlignmentRight) {
				// Pad to the right
				*ps = std::string(size_t(diffSize), padChar) + *ps;
				return;
			} else { // oa==outputAlignmentCenter
				// Pad just half the amount and don't return
				*ps = std::string(size_t(diffSize/2), padChar) + *ps;
			}
		}
	}

	ps->resize(newSize, padChar);
}

// Same as SpecString_ResizeBytes, but sizes and cuts are in characters
static void SpecString_ResizeChars(PSpecString ps, size_t newSize, const std::string& pad, outputAlignment oa, ellipsisSpec es)
{
	size_t len = utf8::charCount(*ps);

	if ((ellipsisSpecNone != es) && (newSize < len) && (newSize >= 3)) {
		size_t prefixLength, suffixLength;
		computeEllipsis(newSize, es, prefixLength, suffixLength);
		*ps = utf8::substr(*ps, 0, prefixLength) + "..." + utf8::substr(*ps, len - suffixLength);
		len = newSize;
	}

	if (oa!=outputAlignmentLeft) {
		int diffSize = ((int)newSize - (int)len);
		if (diffSize == 0) return;
		if (diffSize < 0) {
			if (oa==outputAlignmentRight) {
				*ps = utf8::substr(*ps, size_t(-diffSize));
				return;
			} else { // oa==outputAlignmentCenter
				*ps = utf8::substr(*ps, size_t(-diffSize/2));
				len -= size_t(-diffSize/2);
			}
		} else { // diffSize > 0
			if (oa==outputAlignmentRight) {
				*ps = SpecString_Pad(pad, size_t(diffSize)) + *ps;
				return;
			} else { // oa==outputAlignmentCenter
				*ps = SpecString_Pad(pad, size_t(diffSize/2)) + *ps;
				len += size_t(diffSize/2);
			}
		}
	}

	if (len > newSize) {
		ps->resize(utf8::charToByte(*ps, newSize));
	} else if (len < newSize) {
		*ps += SpecString_Pad(pad, newSize - len);
	}
}

void SpecString_Resize(PSpecString ps, size_t newSize, const std::string& pad, outputAlignment oa, ellipsisSpec es)
{
	MYASSERT(!pad.empty());
	if (!g_bSupportUTF8 || (pad.size() == 1 && utf8::isAscii(*ps))) {
		SpecString_ResizeBytes(ps, newSize, pad[0], oa, es);
	} else {
		SpecString_ResizeChars(ps, newSize, pad, oa, es);
	}
}
