# Advanced ALU

Very much a draft

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
| `frombin(x)` | Returns the decimal value of the binary `x`. For example, if `x` is "A", the function returns 65; if `x` is "AB" the function returns 16961. Binary values are assumed to be in little-endian order. |
| `pow(x,y)` | Returns `x` raised to the power of `y` |
| `sqrt(x)` | Returns the square root of `x` |
| `tobin(x)` | Returns a binary (usually unprintable) representation of the integer number x. For example, if `x` is 65 the function returns "A"; if `x` is 16961 the function returns "AB". |
| `tobine(x,n)` | Returns a binary representation of the integer number x as an *n*-byte string. |

## Table of String Functions
| Function | Description |
| -------- | ----------- |
| `center(s,n)` or `centre(s,n)` | Returns the `n` center-most characters of the string `s`. The result is padded with spaces on both sides if `n` is greater than the length of `s`. |
| `includes(haystack,needle)` | Boolean function. Returns `1` if `needle` is a substring of `haystack`, or `0` otherwise |
| `left(s,n)` | Returns the `n` left-most characters of the string `s`. The result is padded with spaces on the right if `n` is greater than the length of `s`. |
| `len(s)` | Returns the length (in characters) of the string `s` |
| `right(s,n)` | Returns the `n` right-most characters of the string `s`. The result is padded with spaces on the left if `n` is greater than the length of `s`. |
| `substr(s,start,len)` | Returns a substring of `s` starting from offset `start` for `len` characters |
| `pos(needle,haystack)` | Returns the 1-based position of the first occurrence of the substring `needle` in the string `haystack` |
| `rpos(needle,haystack)` | Returns the 1-based position of the *last* occurrence of the substring `needle` in the string `haystack` |

## Table of Record Access Functions
| Function | Description |
| -------- | ----------- |
| `field(n)` | Returns the *n*-th field |
| `fields(n,m)` | Returns the substring from the *n*-th field to the *m*-th field |
| `fieldcount()` | Returns the number of fields in the current record |
| `fieldend(n)` | Returns the offset from the start of the record that the *n*-th field ends at. Like other things in **specs**, this is 1-based. | 
| `fieldindex(n)` | Returns the offset from the start of the record that the *n*-th field starts at. |
| `fieldlength(n)` | Returns the length of the *n*-th field |
| `iterno()` | Returns the number of processing cycles we have already gone through. Unless `READ` or `READSTOP` are used, this will be equal to the number of records read so far. |
| `recno()` | Returns the number of the currently read record. If the `READ` or `READSTOP` keywords are used this may be greater than `iterno()` |
| `record()` | Returns the entire input record |
| `word(n)` | Returns the *n*-th word |
| `words(n,m)` | Returns the substring from the *n*-th word to the *m*-th word |
| `wordcount()` | Returns the number of words in the current record |
| `wordend(n)` | Returns the offset from the start of the record that the *n*-th word ends at. Like other things in **specs**, this is 1-based. | 
| `wordindex(n)` | Returns the offset from the start of the record that the *n*-th word starts at. |
| `wordlength(n)` | Returns the length of the *n*-th word |

## Table of Statistical Pseudo-Functions
| Function | Description |
| -------- | ----------- |
| `sum(a)` | Returns the sum of all values that have been assigned to *field identifier* `a` |
| `min(a)` | Returns the minimum of all values that have been assigned to *field identifier* `a` |
| `max(a)` | Returns the maximum of all values that have been assigned to *field identifier* `a` |
| `avg(a)` | Returns the average (arithmetic mean) of all values that have been assigned to *field identifier* `a` |


## Table of Special Functions
| Function | Description |
| -------- | ----------- |
| `first()` | Returns `1` in the run-in phase, or `0` otherwise |
| `eof()` | Returns `1` in the run-out phase, or `0` otherwise |
| `conf(s)` | Returns the configured string `s` if it exists |
| `tf2d(s,f)` | Returns the time represented by the string in `s` in the format in `f` converted to the **specs** internal format, which is seconds since the UNIX epoch with up to 6 decimal places. The format in `f` is similar to the one for the function `strftime` in C and Python, with the addition of %*x*f to represent fractions of a second with *x* digits. |
| `d2tf(x,f)` | Returns the string representation of the number `x` treated as the internal time format and formatted according to the string in `f`. |




