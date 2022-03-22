#include <string.h>  // for memcpy
#include "processing/Config.h"
#include "ErrorReporting.h"
#include "SpecString.h"



void SpecString_Resize(PSpecString ps, size_t newSize, void* pPadChar, outputAlignment oa, ellipsisSpec es)
{
	char padChar = *((char*)pPadChar);
	SpecString_Resize(ps, newSize, padChar, oa, es);
}

void SpecString_Resize(PSpecString ps, size_t newSize, char padChar, outputAlignment oa, ellipsisSpec es)
{
	if ((ellipsisSpecNone != es) && (newSize < ps->size()) && (newSize >= 3)) {
		size_t totalLength, prefixLength, suffixLength;

		totalLength = newSize - 3;

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
