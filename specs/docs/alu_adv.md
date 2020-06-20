# Advanced ALU

## Table of Operands

| Op | Name | Meaning |
| -- | ---- | ------- |
| `+` | Unary Plus | |
| `-` | Unary Minus | Negates its operand, so if `a` is 5.3 then `-a` is -5.3 |
| `!` | Unary Not | Logical Not. If the result is zero, returns `1`, otherwise returns zero |
| `+` | Binary Plus | Returns the sum of its two operands |
| `-` | Binary Minus | Returns the difference between the left-hand operand and the right-hand operand |
| `*` | Binary Multiplication | Returns the product of its two operands |
| `/` | Binary Division | Returns the quotient from dividing the left-hand operand by the right-hand operand |
| `//` | Binary Integer Division | Returns the integer quotient from dividing the left-hand operand by the right-hand operand |
| `%` | Binary Modulu | Returns the remainder from dividing the left-hand operand (the dividend) by the right-hand operand (the divisor) |
| `\|\|` | Concatenation | Returns the concatenation of its two operands |
| `<` | Logical LT | Returns `1` is the left-hand operand is smaller than the right-hand operand, or `0` otherwise |
| `<=` | Logical LTE | Returns `1` is the left-hand operand is smaller than or equal to the right-hand operand, or `0` otherwise |
| `>` | Logical GT | Returns `1` is the left-hand operand is greater than the right-hand operand, or `0` otherwise |
| `>=` | Logical GTE | Returns `1` is the left-hand operand is greater than or equal to the right-hand operand, or `0` otherwise |
| `<<` | Logical SLT | Returns `1` is the left-hand operand is alphabetically smaller than the right-hand operand, or `0` otherwise. For example, `3` will be smaller than `3.0` |
| `<<=` | Logical SLTE | Returns `1` is the left-hand operand is alphabetically smaller than or equal to the right-hand operand, or `0` otherwise |
| `>>` | Logical SGT | Returns `1` is the left-hand operand is alphabetically greater than the right-hand operand, or `0` otherwise |
| `>>=` | Logical SGTE | Returns `1` is the left-hand operand is alphabetically greater than or equal to the right-hand operand, or `0` otherwise |
| `=` | Logical Equality | Returns `1` if its two operands are equal numerically or string-wise, or `0` otherwise |
| `==` | Strict Equality | Returns `1` if its two operands are equal string-wise, or `0` otherwise |
| `!=` | Logical Inequality | Returns `1` if its two operands are not equal numerically or string-wise, or `0` otherwise |
| `!==` | Strict Equality | Returns `1` if its two operands are not equal string-wise, or `0` otherwise |
| `&` | Logical And | Returns `1` if both of its two operands are non-zero, non-NaN, or `0` otherwise |
| `\|` | Logical Or | Returns `1` if either of its two operands are non-zero, non-NaN, or `0` otherwise |

## Table of Assignment Operators

| Op | Name | Meaning |
| -- | ---- | ------- |
| `:=` | Let | Assigns the value of the right-hand operand to the left-hand counter |
| `+=` | Add | Adds the value of the right-hand operand to the left-hand counter, storing the sum in that counter |
| `-=` | Sub | Subtracts the value of the right-hand operand from the left-hand counter, storing the difference in that counter |
| `*=` | Mul | Multiplies the value of the right-hand operand with the left-hand counter, storing the product in that counter |
| `/=` | Div | Divides the value of the left-hand counter by the right-hand counter, storing the quotient in that counter |
| `//=` | IntDiv | Divides the value of the left-hand counter by the right-hand counter, storing the **integer** quotient in that counter |
| `%=` | RemDiv | Divides the value of the left-hand counter by the right-hand counter, storing the **remainder** in that counter |
| `\|\|=` | Appnd | Appends the string value of the right-hand operand to the string value of the left-hand counter, storing the concatenation in that counter |

## Table of Numerical Functions
| Function | Description |
| -------- | ----------- |
| `abs(x)` | Returns the absolute value of `x` |
| `arccos(x)` | Returns the inverse cosine function, returning an angle expressed in radians |
| `arcdcos(x)` | Returns the inverse cosine function, returning an angle expressed in degrees |
| `arcdsin(x)` | Returns the inverse sine function, returning an angle expressed in degrees |
| `arcdtan(x)` | Returns the inverse tangent function, returning an angle expressed in degrees |
| `arcsin(x)` | Returns the inverse sine function, returning an angle expressed in radians |
| `arctan(x)` | Returns the inverse tangent function, returning an angle expressed in radians |
| `c2u(x)` | Returns an unsigned decimal value, treating x as a binary representation with length from 1 to 8 bytes. **Note:** In *CMS Pipelines* the value of `x` can be up to 108 bits. Here it is capped at 64 |
| `c2d(x)` | Returns an signed decimal value, treating x as a binary representation with length from 1 to 8 bytes and negative numbers in 2's complement |
| `c2f(x)` | Returns a floating-point value, treating x as the platform-native binary representation with the lengths that are valid for the C types `float`, `double`, and `long double` |
| `ceil(x)` | Returns the smallest integer greater than `x` |
| `cos(x)` | Returns the cosine function, treating `x` as an angle expressed in radians |
| `dcos(x)` | Returns the cosine function, treating `x` as an angle expressed in degrees |
| `dsin(x)` | Returns the sine function, treating `x` as an angle expressed in degrees |
| `dtan(x)` | Returns the tangent function, treating `x` as an angle expressed in degrees |
| `exp(x)` | Returns the exponent of `x` |
| `floor(x)` | Returns the largest integer smaller than `x` |
| `fmt(value,format,digits,decimal,separator)` | formats a floating-point `value` as a string. The `format` argument can be omitted, or it can begin with **f** for a **fixed** number of `digits` after the decimal point, or **s** for **scientific** notation. When omitted, the `digits` argument sets the total number of digits displayed. The `decimal` argument sets the character used for the decimal point (default is a period), while the `separator` argument sets the character used as thousands separator (default is none). |
| `frombin(x)` | Returns the decimal value of the binary `x`. For example, if `x` is "A", the function returns 65; if `x` is "AB" the function returns 16961. Binary values are assumed to be in little-endian order. |
| `log(x,base)` | Returns the logarithm of `x`. The default for `base` is to return the natural logarithm. |
| `pow(x,y)` | Returns `x` raised to the power of `y` |
| `rand(x)` | Returns a random value up to and not including the integer `x`. If `x` is omitted, returns a random **real** value between 0.0 up to and not including 1.0 |
| `round(x,d)` | Returns the closest number to `x` that has `d` decimal places. If `d` is omitted, returns the closest integer |
| `sin(x)` | Returns the sine function, treating `x` as an angle expressed in radians |
| `sqrt(x)` | Returns the square root of `x` |
| `tan(x)` | Returns the tangent function, treating `x` as an angle expressed in radians |
| `tobin(x)` | Returns a binary (usually unprintable) representation of the integer number x. For example, if `x` is 65 the function returns "A"; if `x` is 16961 the function returns "AB". |
| `tobine(x,n)` | Returns a binary representation of the integer number x as an *n*-byte string. |

## Table of String Functions
| Function | Description |
| -------- | ----------- |
| `abbrev(h,n,l)` | Returns `1` when the first `l` characters of `n` are equal to the first characters of `h` or `0` otherwise. If `l` is omitted, all of 'n' is considered. |
| `center(s,n)` or `centre(s,n)` | Returns the `n` center-most characters of the string `s`. The result is padded with spaces on both sides if `n` is greater than the length of `s`. |
| `includes(haystack,needle1, [needle2, needle3, needle4])` | Boolean function. Returns `1` if **any** of `needle1`, `needle2`, `needle3`, or `needle4` is a substring of `haystack`, or `0` otherwise. Note that if the `haystack` argument is omitted, it defaults to the entire input line |
| `includesall(haystack,needle1, [needle2, needle3, needle4])` | Boolean function. Returns `1` if **all** of `needle1`, `needle2`, `needle3`, or `needle4` are substrings of `haystack`, or `0` otherwise. Note that if the `haystack` argument is omitted, it defaults to the entire input line |
| `rmatch(string,exp, [matchFlags])` | Returns `1` if the regular expression `regEx` matches `string`, or `0` otherwise. See the special section below for more info on regular expressions. Note that if the `string` argument is omitted, it defaults to the entire input line. See note about `matchFlags` |
| `rsearch(string,exp, [matchFlags])` | Returns `1` if the regular expression `regEx` matches any substring of `string`, or `0` otherwise. See the special section below for more info on regular expressions. Note that if the `string` argument is omitted, it defaults to the entire input line. See note about `matchFlags` |
| `rreplace(string,exp,fmt [matchFlags])` | Returns the string `string`, with all matches of the regular expression `regEx` replaced by what's in `fmt`. See the special section below for more info on regular expressions. Note that if the `string` argument is omitted, it defaults to the entire input line. See note about `matchFlags` |
| `left(s,n)` | Returns the `n` left-most characters of the string `s`. The result is padded with spaces on the right if `n` is greater than the length of `s` |
| `length(s)` | Returns the length (in characters) of the string `s` |
| `right(s,n)` | Returns the `n` right-most characters of the string `s`. The result is padded with spaces on the left if `n` is greater than the length of `s`. |
| `substitute(haystack,needle,subst,max)` | Returns the string `haystack` where occurrences of `needle` have been replaced with the content of the string `subst` for a maximum of `max` times.  The special value **"U"** for `max` indicates that all occurrences of `needle` are to be replaced. The default for `max` is 1 |
| `substr(s,start,len)` | Returns a substring of `s` starting from offset `start` for `len` characters |
| `pos(needle,haystack)` | Returns the 1-based position of the first occurrence of the substring `needle` in the string `haystack`. Note that if the `haystack` argument is omitted, it defaults to the entire input line |
| `lastpos(needle,haystack)` | Returns the 1-based position of the *last* occurrence of the substring `needle` in the string `haystack`. Note that if the `haystack` argument is omitted, it defaults to the entire input line |
| `sfield(str,n,sep)` | This is the equivalent of the `field` function from **CMS Pipelines**. It returns the n-th field, counting from the start of the string (positive *n*) or end of the string (negative *n*), where fields are separated by the first character of the string `sep`. If `sep` is missing or an empty string, the separator is the default one: a tab character |
| `sword(str,n,sep)` | This is the equivalent of the `word` function from **CMS Pipelines**. It returns the n-th word, counting from the start of the string (positive *n*) or end of the string (negative *n*), where words are separated by the first character of the string `sep`. If `sep` is missing or an empty string, the separator is the default one: a space character |

## matchFlags for regular expressions

All three regular expression functions have an argument called `matchFlags`. This is a comma-separated list of matching flags. The flags are given in the following table:

| Flag | Effects | Notes |
|------|---------|-------|
| `not_bol` | Not beginning of line | The first character is not considered a beginning of line. `^` does not match |
| `not_eol` | Not end of line | The last character is not considered an end of line. `$` does not match |
| `not_bow` | Not beginning of word | `\b` does not match as a beginning of word |
| `not_eow` | Not end of word | `\b` does not match as an end of word |
| `any` | Any match | Any match is acceptable |
| `not_null` | Not null | An empty string does not match |
| `continuous` | Continuous | The expression must match a sub-sequence that begins at the first character. Sub-sequences must begin at the first character to match |
| `prev_avail` | Previous Available | One or more characters exist before the first one. (`not_bol` and `not_bow` are ignored) |
| `sed` | sed formatting | For `rreplace` only |
| `no_copy` | No copy | for `rreplace` only - sections that do not match are not copied |
| `first_only` | First only | Only the first occurrence is replaced |


## Table of Other REXX-Derived Functions
| Function | Description |
| -------- | ----------- |
| `bitand(x,y)` | Returns the bit-wise AND of strings `x` and `y`. If they are not of equal length, the length returned is the minimum. |
| `bitor(x,y)` | Returns the bit-wise OR of strings `x` and `y`. If they are not of equal length, the length returned is the minimum. |
| `bitxor(x,y)` | Returns the bit-wise XOR of strings `x` and `y`. If they are not of equal length, the length returned is the minimum. |
| `compare(s1,s2,pad)` | Returns the index of the first mis-matched character, or zero if `s1` and `s2` are equal. If they are of unequal length, the shorter string is padded with the pad character (by default - a space). |
| `copies(string,times)` | Returns the content of `string` repeated `times` times. |
| `delstr(string,start,length)` | Deletes the substring of `string` that starts at position `start` for the specified `length`. If `length` is zero, the rest of the string is deleted from position start to the end. |
| `delword(string,start,length)` | Deletes the substring of `string` that starts at position `start` and is of length `length` blank-delimited words. If `length` is zero, it defaults to removing the rest of the words in `string`. |
| `find(string,phrase)` | Returns the word number of the first occurrence of `phrase` in `string`. Returns 0 if `phrase` is not found. Multiple blanks between words are treated as one in comparisons. |
| `index(haystack,needle,start)` | Returns the character position of `needle` within string `haystack`. Returns 0 if `needle` is not found. If positive, `start` tells where in `haystack` to initiate the search. It defaults to 1 if not specified. The standard `pos` function should be used instead of index if possible. |
| `insert(string,target,position,length,pad)` | Inserts `string` into `target` at position `position` and truncated or padded with `pad` characters to length `length`. With default or zero values, `position` inserts the string at the start of `target`, and the length of the `string` is kept as is. The pad character defaults to space |
| `justify(string,length,pad)` | Evenly justifies words within `string`. The `length` specifies the length of the returned string, while `pad` specifies what padding (by default a space) to insert (if necessary). |
| `overlay(string1, string2 ,start ,length ,pad)` | Returns a copy of `string2`, partially or fully overwritten by `string1`. `start` specifies the starting position of the overlay. `length` truncates or pads `string1` prior to the operation, using `pad` as the pad character. |
| `reverse(string)` | Returns a copy of a `string` with its characters reversed. |
| `sign(number)` | Returns 1 if the `number` is positive, 0 if the `number` is 0, and -1 if the `number` is negative. |
| `space(string,length,pad)` | Formats a `string` by replacing internal blanks with `length` occurrences of the `pad` character. The default pad character is blank and the default length is 1. Leading and trailing blanks are always removed. If `length` is 0, all blanks are removed. |
| strip(string,option,char) | Returns `string` stripped of leading and/or trailing blanks or any other `char` specified. `Option` values determine the action: *L* for leading, *T* for trailing, and *B* for both (the default) |
| `subword(string,start,length)` | Returns the substring that begins at blank-delimited word `start`. If `length` is omitted, it defaults to the remainder of the string. |
| translate(string,tableout,tablein,pad) | Returns a translated copy of `string`. Characters are translated according to the input translation table `tablein` and its output equivalent, `tableout`. If `tablein` and `tableout` are not coded, all characters in `string` are translated to uppercase. If `tableout` is shorter than `tablein`, it is padded with the `pad` character or its default, blanks. |
| `verify(string, reference ,option ,start)` | Verifies that all characters in `string` are members of the `reference` string. Returns the position of the first character in `string` that is not in `reference`, or 0 if all characters in `string` are in `reference`. <br />`start` specifies where in `string` to start the search, the default is 1. The `option` may be:<ul><li> **N** (Nomatch) — Default. Works as described earlier.</li><li> **M** (Match) — Returns the position of the first character in string that is in reference.</li></ul> |
| `wordindex(string,wordno)` | Returns the character position of the first character of the blank-delimited word given by word number `wordno` within `string`. Returns 0 if the word numbered `wordno` does not exist in the `string`.|
| `wordlength(string,wordno)` | Returns the length of the blank-delimited word given by word number `wordno` within `string`. Returns 0 if the word numbered `wordno` does not exist in the `string`.|
| `wordpos(phrase, string [,start])` | If `phrase` is a substring of `string`, returns the word number position at which it begins. Otherwise returns 0. `start` is an optional word number within `string` at which the search starts. It defaults to 1. |
| `words(string)` | Returns the number of blank-delimited words within the `string`.|
| `xrange(start,end)` | Returns a string composed of all the characters between `start` and `end` inclusive. `start` defaults to 0x00, and `end` defaults to 0xff. |
| `x2d(string,length)` | Returns the value in `string` converted to decimal. If `length` is missing or non-positive, the resulting decimal is signed, otherwise it is unsigned. |

## Table of Record Access Functions
| Function | Description |
| -------- | ----------- |
| `field(n)` | Returns the *n*-th field |
| `fieldrange(n,m)` | Returns the substring from the *n*-th field (default first) to the *m*-th field (default last) |
| `fieldcount()` | Returns the number of fields in the current record |
| `fieldend(n)` | Returns the offset from the start of the record that the *n*-th field ends at. Like other things in **specs**, this is 1-based. | 
| `fieldindex(n)` | Returns the offset from the start of the record that the *n*-th field starts at. |
| `fieldlength(n)` | Returns the length of the *n*-th field |
| `number()` | Returns the number of processing cycles we have already gone through. Unless `READ` or `READSTOP` are used, this will be equal to the number of records read so far. |
| `range(n,m)` | Returns the substring from the *n*-th character (default first) to the *m*-th character (default last) |
| `recno()` | Returns the number of the currently read record. If the `READ` or `READSTOP` keywords are used this may be greater than `number()` |
| `record()` | Returns the entire input record |
| `word(n)` | Returns the *n*-th word |
| `wordrange(n,m)` | Returns the substring from the *n*-th word (default first) to the *m*-th word (default last) |
| `wordcount()` | Returns the number of words in the current record |
| `wordend(n)` | Returns the offset from the start of the record that the *n*-th word ends at. Like other things in **specs**, this is 1-based. | 
| `wordstart(n)` | Returns the offset from the start of the record that the *n*-th word starts at. |
| `wordlen(n)` | Returns the length of the *n*-th word |

## Table of Statistical and Frequency Map Pseudo-Functions
| Function | Description |
| -------- | ----------- |
| `present(a)` | Returns `1` if the *field identifier* `a` is assigned, or `0` otherwise |
| `sum(a)` | Returns the sum of all values that have been assigned to *field identifier* `a` |
| `min(a)` | Returns the minimum of all values that have been assigned to *field identifier* `a` |
| `max(a)` | Returns the maximum of all values that have been assigned to *field identifier* `a` |
| `average(a)` | Returns the average (arithmetic mean) of all values that have been assigned to *field identifier* `a` |
| `variance(a)` | Returns the variance (the expectation of the squared deviation of a random variable from its mean) of the values that have been assigned to *field identifier* `a` |
| `stddev(a)` | Returns the standard deviation (the square root of the variance) of the values that have been assigned to *field identifier* `a` |
| `stderrmean(a)` | Returns the [standard error](https://en.wikipedia.org/wiki/Standard_error) (the standard deviation divided by sample size minus 1) of the values that have been assigned to *field identifier* `a` |
| `fmap_nelem(a)` | Returns the number of distinct values of *field identifier* `a` |
| `fmap_nsamples(a)` | Returns the number of samples collected of *field identifier* `a` |
| `fmap_common(a)` | Returns the string value with most occurrences of *field identifier* `a`. In case of a tie, one of the values is returned. |
| `fmap_rare(a)` | Returns the string value with least but non-zero occurrences of *field identifier* `a`. In case of a tie, one of the values is returned. |
| `fmap_count(a,s)` | Returns the number of occurences of string `s` in *field identifier* `a` |
| `fmap_frac(a,s)` | Returns the fraction of *field identifier* `a` values that are equal to `s` |
| `fmap_pct(a,s)` | Returns the percentage of *field identifier* `a` values that are equal to `s` |
| `fmap_sample(a,s)` | Treats the value of the string in `s` as a new sample for *field identifier* `a`. Returns the count of occurences of `s`. This is useful mainly in tests. |
| `fmap_dump(a,format,sortOrder,showPct)` | Returns a string containing a dump of the frequency map for *field identifier* `a`. |
| `countocc(n,h)` | Returns the number of matches for this particular needle so far |
| `countocc_get(n)` | Returns the number of matches for this particular needle so far. No match is made here |
| `countocc_dump(format,sortOrder,showPct)` | Returns a string containing a dump of the frequency map used in `countocc` |

The parameters for the `fmap_dump` functions are as follows:
* *format*. Possible values:
  * *txt* or *0* or empty string: textual representation of string and count, with the field width adjusted to fit the largest value of the field. This is the default.
  * *lin*: same as *txt*, but surrounded by lines.
  * Integer value: same as *txt*, but the width of the string field is fixed.
  * *csv*: a Commad Seperated Value dataset.
  * *json*: a JavaScript Object Notation dataset.
* *sortOrder*. Possible values:
  * *s*, *sa*, or empty string: Sort by ascending alphabetical order of the string key. This is the default.
  * *sd*: Sort by descending alphabetical order of the key.
  * *c* or *ca*: Sort by ascending count numbers -- from least common to most common.
  * *cd*: Sort by descending count numbers -- from most common to rarest.
* *showPct* - evaluated as boolean. If *true* causes the textual formats to print out a percentage. Causes the CSV and JSON formats to add a fraction. Default is *false*.


## Table of Special Functions
| Function | Description |
| -------- | ----------- |
| `first()` | Returns `1` in the run-in phase, or `0` otherwise |
| `eof()` | Returns `1` in the run-out phase, or `0` otherwise |
| `conf(key,default)` | Returns the configured string `key` if it exists, the value `default` if it doesn't, and **NaN** if `default` is omitted |
| `defined(key)` | Returns `1` if the configured string `key` is defined, or `0` if it isn't |
| `tf2mcs(s,f)` | Returns the time represented by the string in `s` in the format in `f` converted to the **specs** internal format, which is microseconds since the UNIX epoch. The format in `f` is similar to the one for the function `strftime` in C and Python, with the addition of %*x*f to represent fractions of a second with *x* digits. |
| `mcs2tf(x,f)` | Returns the string representation of the number `x` treated as the internal time format and formatted according to the string in `f`. |
| `tf2s(s,f)` | Returns the time represented by the string in `s` in the format in `f` converted to seconds since the UNIX epoch. The format in `f` is similar to the one for the function `strftime` in C and Python, with the addition of %*x*f to represent fractions of a second with *x* digits. |
| `s2tf(x,f)` | Returns the string representation of the number `x` treated as seconds since the UNIX epoch and formatted according to the string in `f`. |
| `string(x)` | Returns the same value as the argument, but forced to be stored as a string. Such a value can still be evaluated as a number, so `string(3)+2` evaluates to `5`. |
| `next()` | Returns the index of the print position. `w1 "(next())"` should do the same as `w1 next`. |




