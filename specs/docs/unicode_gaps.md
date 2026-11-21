# Gaps in UTF8 Mode

This lists the places where UTF8 mode (`-s utf8=1`, `SPECS_UTF8=1` or `+UTF8 on`) is incomplete. The entries come from reading the sources and from running the examples below against `exe/specs`. Every example assumes UTF8 mode is on.

Severity:

- **Error**: specs aborts or rejects valid input.
- **Wrong output**: specs gives a wrong result without warning. This includes corrupted UTF-8.
- **Inconsistent**: two features that should agree give different answers.
- **Missing**: specs does not support the feature, and says so.

## Regular expressions

| Severity | Gap |
|----------|-----|
| Missing | `rmatch`, `rsearch` and `rreplace` work on bytes. `.` matches one byte, not one character, and a character class cannot contain a non-ASCII character. Specs prints a warning, which `regexWarn=0` suppresses. |
| Wrong output | A match can start or end inside a multi-byte character. `rreplace(record(),"^.","X")` on `éa` returns `X` followed by the stray byte `\xa9`, followed by `a`. |
| Missing | The regex engine is byte-based, so case-insensitive matching cannot be expected to fold non-ASCII letters (not tested). |

## Case conversion (`lcase`, `ucase`, the `LCASE`/`UCASE` conversions) - FIXED (issue #574)

`conv_LCASE` and `conv_UCASE` in `processing/conversions.cc` use simple one-to-one case mapping. They throw if the mapped character has a different UTF-8 length.

| Severity | Gap | Example |
|----------|-----|---------|
| Error | `Unexpected length discrepancy` for any character whose case mapping changes its byte length. | `lcase` of `İ` (U+0130); `ucase` of `ß` and `ſ` (U+017F) |
| Wrong output | The loop stops at the first invalid UTF-8 byte, so the rest of the string is left unconverted. | |
| Missing | There is no full or special-casing mapping (`ß` to `SS`), no locale-specific mapping (Turkish dotted and dotless i), and no context-sensitive mapping (Greek final sigma). | |

## ALU string functions that still work on bytes

| Severity | Function | Gap |
|----------|----------|-----|
| ~~Wrong output~~ fixed | `translate` | Now works on graphemes in UTF8 mode. The sequential-replacement semantics of the byte version are kept (so `translate('ab','ba','ab')` gives `aa`, unlike REXX). |
| ~~Wrong output~~ fixed | `strip` | Now works on graphemes in UTF8 mode. |
| Inconsistent | `delword`, `subword`, `wordindex`, `wordlength` (the `breakIntoWords*` helpers) | They split words with the C `isspace()` on bytes. U+00A0 and U+3000 are not separators, although the default word separator for `w1`, `w2` and so on does treat them as separators (see *Whitespace* below). |
| Inconsistent | `wordcount`, `fieldcount` with an explicit separator | An ASCII-only separator argument is required in UTF8 mode, and the call fails on a non-ASCII one. `wordcount` without a separator, which is the common case, does not apply the Unicode whitespace rules. |

## Whitespace and word splitting

`a<U+00A0>b` is a single word for `wordcount(record())`, `subword`, `delword` and `wordlength`. It is two words for the `w1` / `w2` word ranges. The cause is that the record-level splitter (`identifyWordsUTF8`) uses `utf8::isSpace`, while the ALU functions above use byte-wise `isspace`.

## Comparison and equality

| Severity | Gap |
|----------|-----|
| Inconsistent | `==`, `!=`, `<`, `>`, `<=`, `>=` and the strict operators compare raw bytes. `e` + U+0301 and U+00E9 both have `length()` 1 and look identical, but `record()=="é"` is 0. The `compare`, `abbrev` and `wordpos` functions normalise each grapheme to NFC (`canonicalGrapheme`), so they treat the two as equal. |
| Missing | The ordering operators give code point order, not a collation order. This is probably acceptable, but it should be documented. |

## Record I/O

| Severity | Gap |
|----------|-----|
| ~~Wrong output~~ fixed | Fixed record formats (`--recfm F`, `--recfm FD`, `--lrecl n`) now cut records at *n graphemes* in UTF8 mode, not *n bytes*. A multi-byte character is never split across two records. `printf 'aébcd\n' \| specs -s utf8=1 --recfm F --lrecl 2 1-* 1` now yields `aé`, `bc` and `d\n` (the newline is data, as in fixed format). |

All widths are in graphemes in UTF-8 mode. This is now implemented: `--lrecl 80` means that records are
80 bytes in ASCII mode, and 80 graphemes in UTF-8 mode. 

## Identifiers and tokenizer

| Severity | Gap |
|----------|-----|
| Missing | `mayBeNamedString` in `cli/tokens.cc` accepts only ASCII letters, digits and `_` for named strings, so a name containing a non-ASCII letter, such as `été`, cannot be a named string (from reading the code; not tested). |

## Width

Widths count grapheme clusters, not terminal columns. CJK and fullwidth characters take two columns, so tabular output does not line up. The guidebook already documents this. It is listed here because no feature offers a way to count columns.

## Suggested priority

1. Case conversion errors. They abort the run on valid input, and the fix is local. (solved)
2. `translate` and `strip`, where the output can be invalid UTF-8. (solved)
3. ~~Fixed-length records, where the output can be invalid UTF-8.~~ (solved)
4. A single definition of whitespace for the ALU word functions and the record-level splitter.
5. Normalisation in the comparison operators, or documentation of the difference.
6. Regular expressions. This is the largest change and needs a decision on the engine, for example ICU or PCRE2 with UTF support, versus `std::regex` over code points.
