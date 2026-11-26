# Gaps in UTF8 Mode

This lists the places where UTF8 mode (`-s utf8=1`, `SPECS_UTF8=1` or `+UTF8 on`) is incomplete **now**. Entries that were fixed are not kept. The entries come from reading the sources and from running the examples below against `exe/specs`. Every example assumes UTF8 mode is on.

Severity:

- **Error**: specs aborts or rejects valid input.
- **Wrong output**: specs gives a wrong result without warning. This includes corrupted UTF-8.
- **Inconsistent**: two features that should agree give different answers.
- **Missing**: specs does not support the feature, and says so.

## Regular expressions (known, left open)

`rmatch`, `rsearch` and `rreplace` use a byte-based engine (`std::regex` over `char`).

| Severity | Gap | Example |
|----------|-----|---------|
| Missing | `.` matches one byte, not one character, and a character class cannot contain a non-ASCII character. Specs prints a warning, which `regexWarn=0` suppresses. | `rmatch("éa","^.a")` gives `0`; `rmatch("éa","^[é]a")` gives `0` |
| Wrong output | A match can start or end inside a multi-byte character, so the result can be invalid UTF-8. | `rreplace("éa","^.","X")` returns `X`, the stray byte `\xa9`, then `a` |
| Missing | Case-insensitive matching does not fold non-ASCII letters. | |
| Missing | Matching does not canonicalize, so a composed character does not match its decomposed spelling, unlike `pos` and the comparison operators. | |

## Word and field separators in records

`WORDSEPARATOR`, `FIELDSEPARATOR` (`FS`) and the word/field splitters (`identifyWordsUTF8`, `identifyFieldsUTF8` in `processing/ProcessingState.cc`) compare the *first code point* of each grapheme with the set of code points of the separator.

| Severity | Gap | Example |
|----------|-----|---------|
| Wrong output | A grapheme whose base character is a separator is treated as that separator, so the grapheme is split. | `WORDSEPARATOR e` on `ae` + U+0301 + `b cd` splits after `a`, though `é` is not `e` |
| Inconsistent | Separators are not compared in canonical form. A composed separator does not match the decomposed text, and the reverse. The ALU functions (`field`, `fieldcount`, `wordcount` and the like) do canonicalize. | `FS é` (U+00E9) on `ae` + U+0301 + `b` does not split; `FS` e + U+0301 on `aéb` (U+00E9) does not split |

## ALU functions that work on bytes

| Severity | Function | Gap |
|----------|----------|-----|
| Wrong output | `xrange` | Works on bytes, and uses only the first byte of each argument. `xrange("a","é")` returns the bytes `a`..`\xc3`, which is invalid UTF-8, and `xrange("a")` (default end `0xff`) returns invalid UTF-8 above `~`. |
| Not a gap | `c2d`, `c2u`, `c2f`, `frombin`, `bswap`, `c2x`, `x2ch`, `x2d`, `d2x` | Operate on the bytes of the UTF-8 encoding, which is what a binary conversion should do. Only `x2ch` and `bswap` can produce invalid UTF-8 from valid input. |

## Conversions

| Severity | Gap | Example |
|----------|-----|---------|
| Error | `ROT13` aborts the run on any non-ASCII input, instead of passing non-letters through. | `specs '1-* ROT13 1'` on `éa` fails with `ROT13 only applies to ASCII strings` |
| Not a gap | `C2B`, `C2X`, `B2C`, `X2CH`, `D2X`, `X2D` and `BSWAP` work on the bytes of the encoding, like the ALU byte functions above. | |

## Case conversion (`lcase`, `ucase`, the `LCASE`/`UCASE` conversions)

Simple one-to-one case mapping, one code point at a time (`utf8proc_tolower`, `utf8proc_toupper`). Lengths that change are handled.

| Severity | Gap | Example |
|----------|-----|---------|
| Missing | No full or special-casing mapping. | `ucase("ß")` gives `ẞ` (U+1E9E), not `SS`; `lcase("İ")` gives `i`, not `i` + U+0307 |
| Missing | No locale-specific mapping (Turkish dotted and dotless i). | |
| Missing | No context-sensitive mapping (Greek final sigma). | |

## Record input

| Severity | Gap |
|----------|-----|
| Missing | A UTF-8 byte order mark (U+FEFF) at the start of the input is not stripped. It is data, one character long, and `1-* 1` writes it back. |
| Missing | UTF-16 and UTF-32 input, and other encodings, are not recognised or converted. Input is UTF-8, or invalid bytes that pass through unchanged. |

## Platform

| Severity | Gap |
|----------|-----|
| Missing | The sources contain no Windows code-page or wide-character handling (`CP_UTF8`, `SetConsoleOutputCP`, `wmain`). Non-ASCII command-line arguments and console output on Windows are therefore not supported by specs itself and depend on the terminal and run-time settings. (Not tested on Windows.) |

## Not gaps

- Name of a persistent variable is ASCII by design (`pset`, `pget`, `pdefined`, `pclear`, `#name`).
- Comparison operators order graphemes by code point, not by collation. This is documented; a collation algorithm would be new functionality (see `uca_collation_function.md`).
- Lengths and positions count grapheme clusters, not terminal columns. `utf8width` and `utf8substr` give columns.

## Suggested priority

1. Regular expressions. This is the largest change and needs a decision on the engine, for example ICU or PCRE2 with UTF support, versus `std::regex` over code points.
2. Word and field separators in records: compare whole graphemes in canonical form, as the ALU functions do.
3. `ROT13` aborting on non-ASCII input.
4. `xrange` in UTF8 mode: either a code-point range, or a documented restriction.
5. BOM handling and platform notes.
