# Advanced ALU

Very much a draft

## Table of Operands

| Op | Name | Meaning |
| -- | ---- | ------- |
| `+` | Unary Plus | |
| `-` | Unary Minus | Negates its operand, so if `a` is 5.3 then `-a` is -5.3 |
| `!` | Unary Not | Logical Not. If the result is zero, returns `1`, otherwise returns zero |
| `+` | Binary Plus | Returns the addition of its two operands |
| `-` | Binary Minus | Returns the difference between the left-hand operand and the right-hand operand |
| `*` | Binary Multiplication | Returns the product of its two operands |
| `/` | Binary Division | Returns the quotient from dividing the left-hand operand by the right-hand operand |
| `//` | Binary Integer Division | Returns the integer quotient from dividing the left-hand operand by the right-hand operand |
| `%` | Binary Modulu | Returns the remainder from dividing the left-hand operand (the dividend) by the right-hand operand (the divisor) |
| `||` | Concatenation | Returns the concatenation of its two operands |
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
| `!=` | Logical Equality | Returns `1` if its two operands are not equal numerically or string-wise, or `0` otherwise |
| `!==` | Strict Equality | Returns `1` if its two operands are not equal string-wise, or `0` otherwise |
| `&` | Logical And | Returns `1` if both of its two operands are non-zero, non-NaN, or `0` otherwise |
| `|` | Logical Or | Returns `1` if either of its two operands are non-zero, non-NaN, or `0` otherwise |

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
| `||=` | Appnd | Appends the string value of the right-hand operand to the string value of the left-hand counter, storing the concatenation in that counter |

## Table of Numerical Functions
| Function | Description |
| -------- | ----------- |
| `abs(x)` | Returns the absolute value of `x` |
| `pow(x,y)` | Returns `x` raised to the power of `y` |
| `sqrt(x)` | Returns the square root of `x` |

## Table of String Functions
| Function | Description |
| -------- | ----------- |
| `len(s)` | Returns the length (in characters) of the string `s` |

## Table of Record Access Functions
| Function | Description |
| -------- | ----------- |
| `record()` | Returns the entire input record |

## Table of Special Functions
| Function | Description |
| -------- | ----------- |
| `first()` | Returns `1` in the run-in phase, or `0` otherwise |
| `eof()` | Returns `1` in the run-out phase, or `0` otherwise |




