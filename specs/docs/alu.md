# Arithmetic-Logical Unit

The Arithmetic-Logical Unit (**ALU**) is a part of **specs** that allows you to perform string and numerical calculations on values derived from the input records and other sources.

The ALU allows you to form **expressions** to be used in conditional statements (see [program structure](struct.md) -- currently a dead link).  For example, the following does exactly what you would expect:
`specs print "2+3" 1`

## Elements of the ALU 

### String and Numerical Literals
These are string or numerical values that appear in the expression.  The numbers `2` and `3` in the above example are such literals. The ALU is not at all strongly typed, so the following is entirely valid and produces the output `2+3=5`
```
specs print "'2+3=' || (2+3)" 1
``` 
### Field Identifiers
The field identifiers mentioned in [Basic Specifications](basicspec.md) can be used as values in expressions by using just the single letter. For example, here's an expression involving a field identifier:
```
specs  a: word 1 .  PRINT "'The first word is ' || a" 1
```
| Input | Output |
| ----- | ------ |
| Hello there | The first word is Hello |
| My, it's been a long long time | The first word is My, |

You can test whether a field identifier is assigned using the boolean function `present()`

### Counters
Counters are like variables in programming languages. Counters can be assigned in `SET` *spec units* (see below) and read within expressions. Counters are numbered and preceded by a hash sign as in the following example:
```
specs  a: word 1 1  "items. Total is" nextword  SET "#0+=a"  PRINT "#0" nextword
```
| Input | Output |
| ----- | ------ |
| 5 | 5 items. Total is 5 |
| 7 | 7 items. Total is 12 |
| 3 | 3 items. Total is 15 |

All counters are initialized to the value zero (0) before they are first set.

### Configured Literals
If you have literals that you use a lot, you can place them in the `.specs` file located in your home directory if you are using a POSIX-based OS (such as Mac OS or Linux), or the specs.cfg file located in your Windows home directory. Here's an example of such a file:
```
pi: 3.14159265
favoriteAnimal: cat
billion: 1000000000
timezone: Asia/Bangkok
locale: en_US
Motto: "memento mori"
```
Configured literals can also be set using the command-line switches `--set` or `-s`.
So let's use them in a specification:
```
REQUIRES pi
specs r: word 1 .
         /Tau is/ 1
         PRINT "@pi*2" nextword
         /; Circle area is/ next
         PRINT "@pi*r*r" nextword
         /; My favorite animal is a/ next
         PRINT "@favoriteAnimal" nextword
         /; My motto is:/ next
         PRINT "@Motto" nextword
```
| Input | Output |
| ----- | ------ |
| 5 | Tau is 6.2831853; Circle area is 78.53981625; My favorite animal is a cat; My motto is: memento mori |
| 7 | Tau is 6.2831853; Circle area is 153.93803985; My favorite animal is a cat; My motto is: memento mori |
| 3 | Tau is 6.2831853; Circle area is 28.27433385; My favorite animal is a cat; My motto is: memento mori |

**Note:** The specification above was written as in a file. When used on the Unix command-line, those quotes around the string configured literals are considered to be marking off a single argument. For example, the following specification:
```
specs PRINT "@favoriteAnimal" 1
```
is interpreted as 
```
specs PRINT @favoriteAnimal 1
```
Configured literals work also outside of expressions, so that is the same as 
```
specs PRINT /cat/ 1
```
which generates an error:
```
Error while parsing command-line arguments: Error in expression in Token PRINT at index 1 with content <cat>
```
What you want to do instead is 
```
specs PRINT "'@favoriteAnimal'" 1
```
which gives you
```
cat
```

There are also pre-defined values that behave like a configured literals. One is `@version`. It returns the current version of specs. Try it:
```
specs @version 1
```

Others are `@cols`, which contains the number of columns in the terminal screen, and `@rows`, which contains the number of rows on that same screen.

Additionally, the `@@` string stands for the entire input record.

`@python` contains either "Enabled" or "Disabled" depending on whether python function support is enabled.

**Note:** The timezone used in the date conversion can also be set in the configuration file with a `timezone` entry. Similarly, the locale used can be set with a `locale` entry.

**Note:** Some *specifications* as shown above may depend of a specific configured literal being defined. It may be prudent to have such specifications fail quickly by using the `REQUIRES` keyword as shown above. 

### Operators
The ALU supports many of the operators available in the *REXX* programming languages. This includes addition, subtraction, logical comparisons, and string concatenation. 
There are three division operands:

| Op | Name | Meaning | Example |
| -- | ---- | ------- | ------- |
| `/` | Division | returns the quotient of dividing two numbers | 17 / 4 ==> 4.25 |
| `//` | Integer Division | returns the integer quotient of two numbers | 17 // 4 ==> 4 |
| `%` | Remainder or Modulu | returns the remainder in division | 17 % 4 ==> 1 |

**Note:** The roles of the `//` and `%` operators is reversed compared to *CMS Pipelines*. The reason for this is that CMS Pipelines was written for people who know the *REXX* language, where the `//` is the remainder operand and `%` is the integer division. Unix and Windows users are more accustomed to languages like C/C++, Java, Javascript and Python, where `%` is the remainder operator.

A full list of supported operators can be found in [Advanced ALU Topics](alu_adv.md).

### Functions
The specs ALU has a bunch of built-in functions. The full list is available at [Advanced ALU Topics](alu_adv.md), but here are a few examples:
* len(x) - returns the length of x considered as a string
* record() - returns the entire input record
* words(start, count) - returns a substring of the input record, similar to what `words start.count` would yield in a data field.
* tf2mcs(s,f) and mcs2tf(x,f) - convert a formatted date string to the internal representation, which is measured in microseconds since the Unix epoch (1-Jan-1970 at midnight), and convert the other way.  The format is similar to that of the C function strftime(), plus %xf for fractional seconds, where x represents number of digits from 0 to 6.
* pos(needle,haystack)
* includes(hatstack,needle)

Examples:
```
tf2mcs('2019-01-03 23:23:23','%Y-%m-%d %H:%M:%S') ==> 1546550603000000
len(743) ==> 3
left(743,2) ==> 74
```

## SET
Counters are set through the *SET* spec unit. The string that follows the SET token consists of a counter, an assignment operator, and an expression. For example:
`#0 := #2 - 5`
The full list of operators is in [Advanced ALU Topics](alu_adv.md). The most common ones are the regular assignment operator: `:=`, and those derived from binary operators such as `+=`, `*=` etc.

### Assignments as Expressions
Assignment statements can also be used in expressions. Wherever an expression is required, an assignment statement can be used. It is performed, and the expression evaluates to the final result in the counter. For example:
```
   specs PRINT "#0:=2"    1
         /plus/           nextword
         PRINT "#1:=3"    nextword
         /equals/         nextword
         PRINT "#1+=#0"   nextword
```
will output: `2 plus 3 equals 5`

**Note:** Using assignments as expressions has the side effect of setting the counter. With conditional specifications (see [Structured Specification](struct.md)) some expressions are never evaluated. When assignments are used for such expressions, they will not be performed and the counter will not be altered.
