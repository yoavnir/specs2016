---
title: "specs: The Complete Guidebook"
author: 
  - "Yoav Nir"
  - "Devin (AI)"
documentclass: "book"
classoption:
  - openany
  - A4paper
mainfont: "DejaVu Serif"
monofont: "DejaVu Sans Mono"
toc: true
toc-depth: 1
numbersections: false
keywords: 
  - "specs"
  - "guidebook"
  - "AI-generated"
date: "XXDATE"
geometry: "left=0.75in,right=0.75in,top=0.75in,bottom=0.75in"
header-includes: |
  \usepackage{etoolbox}
  \pretocmd{\chapter}{\clearpage}{}{}
include-before: |
  \begin{titlepage}
  \centering
  {\Huge\bfseries\itshape specs\par}
  \vspace{0cm}
  {\Huge The complete guidebook\par}
  \vspace{2cm}
  {\Large\itshape\color{blue}\underline{https://github.com/yoavnir/specs2016}\par}
  \vfill
  \includegraphics[width=12cm]{XXDOCS/resources/specs-logo.png}
  \vfill
  {\Large Version XXVERSION\par}
  \vspace{1cm}
  {\Large XXDATE\par}
  \end{titlepage}
  \clearpage
  \chapter*{Preface}
  \textit{This book is a tutorial and reference for the} \textbf{specs} \textit{text-processing utility}
  \bigskip\par
  This guidebook teaches you \textbf{specs} from the ground up. It assumes no prior knowledge of the tool, though familiarity with the Unix command line is helpful, and familiarity with the \textbf{Python} language enables some very powerful \textit{specifications}. 
  
  By the end you will be able to write specifications ranging from one-liners that reformat a column of numbers to multi-page programs that join files, compute statistics, and incorporate Python-language functions.
  \bigskip\par
  \textbf{How to use this book}
  \begin{itemize}
  \item Start with Appendix C to install \textbf{specs}, whether from a pre-built package or by building from source.
  \item Read Chapters 1--5 to understand the mental model, learn the basic \textit{spec units}, and where you can give \textbf{specs} its instructions.
  \item Read Chapters 6--13 to master every feature of the tool.
  \item Read Chapter 14 when you need to \textbf{extend} specs with \textbf{Python} functions that you write by yourself.
  \item Use Chapter 15 for some examples, and Appendices A--B as a desk reference.
  \end{itemize}
  It is best to first install \textbf{specs} on a \textbf{Mac} or \textbf{Linux} machine, although \textbf{specs} works just fine on \textbf{Microsoft Windows} as well. Every example in this book can be run, and it is \textit{recommended} to try things as you learn. The examples assume a POSIX shell (bash for Linux or zsh for Mac OS), although many work in the Microsoft Windows command line environments, both \textbf{cmd.exe} and \textbf{PowerShell}. Where shell-quoting matters it is called out explicitly.
  \clearpage
---

# Chapter 1: Introduction and Mental Model

## What is specs?

**specs** is a command-line utility for parsing and re-arranging text. Its name comes from "specifications" — you describe *what you want done* rather than *how to do it* imperatively. Think of it as a more powerful version of `awk`, one that also handles multi-record aggregation, time conversion, regular expressions, statistics, and arithmetic.

**specs** was originally a **stage** in the **CMS Pipelines** system on IBM mainframes running **VM/ESA** and later **z/VM**. This version is a re-implementation for Linux, Mac OS, and Windows, liberally extended with new features, and with many (but not all!) of the "Mainframe-isms" replaced with "UNIX-isms". As an example, **REXX** integration was replaced with **Python** integration. It does, however, keep the base-1 indexing, meaning that `WORD 1` is the first word in the record.

### What problems does specs solve?

Here are the kinds of questions specs was built to answer:

- *"I have a CSV file. Give me columns 2 and 5, tab-separated, in a fixed-width column."*
- *"Parse this web-server log and give me just the URL and HTTP status code."*
- *"Read all these numbers and print their sum at the end."*
- *"I have two files with matching row counts — join the third column of each."*
- *"Look at adjacent records and flag any two consecutive entries that differ by more than 10."*
- *"Run this command for every line in the file."*

This book assumes that **specs** is installed on your system. If it isn't yet, see **[Appendix C: Installation](#appendixc)** for step-by-step instructions covering both pre-built packages and building from source. Once it's installed, a quick way to check that everything is in order — including whether Python support is available — is:

```
specs @platform
```

which prints something like this:

```
POSIX (darwin) system using the g++ compiler and Python 3.9.6 - release variation
```

### Terminology

The program itself is called **specs**, although the original CMS Pipelines stage could be shortened to **spec**. The arguments to the program form a **specification**, whether that is given on the command line or in a file.
The act of giving the program instructions - of writing a *specification* - is called **specifying**.
So if you've written a *specification* that converts all the FROM/IN/ON/AT fields of a TZDATA file to seconds-since-the-epoch, you have *specified* to make that conversion.

\newpage
## The Mental Model

Understanding three concepts unlocks everything else:

### Records

**specs** is a record-oriented processor. It reads the input one *record* at a time (normally one line at a time), runs your specification against that record, and emits zero or more output records. This cycle repeats until the input is exhausted. For most purposes a record is simply one line of text, but specs lives up to its Mainframe heritage by also handling fixed-length records and streams Where records are delimited by characters other than newline — see **[Record Formats](#recfm)** in **[Chapter 6](#chap6)**.

```
Input stream          specs                Output stream
─────────────         ──────────────────   ─────────────
line 1           →    [specification] →    result 1
line 2           →    [specification] →    result 2
line 3           →    [specification] →    result 3
…                →    [specification] →    …
```

This default one-in/one-out pattern can be broken: you can emit multiple output records per input record, read multiple input records per cycle, or suppress output entirely. These capabilities are covered in [Chapter 12](#chap12).

### The Specification

The *specification* is the set of instructions you give specs. It describes what to extract from the input record, how to transform it, and where to place it in the output record.

A specification consists of **spec units** — small building blocks that each perform one action. The most common spec unit is the **data field**, which copies a piece of the input to a piece of the output. Data fields are described in the **[Spec Units and Data Fields](#datafield)** section of Chapter 2.

### Where Specs Lives in a Pipeline

**specs** is designed to sit in a shell pipeline:

*some-command* | `specs [switches] [spec-units]` | *next-command*


Input can come from three places: 

1. **Standard input**, which by default feeds the primary input stream. 
1. A file, specified with `-i` or `--inFile` for the primary input stream, or `--is2`, `--is3` ... `--is8` for secondary input streams.
1. A command output, specified with `-C` or `--inCmd`, which feeds the primary input stream.

Output can go to four places:

1. **Standard output**, which is the default output stream.
1. A file, specified with `-o` or `--outfile` for the primary output stream, or `--os2`, `--os3` ... `--os8` for secondary output streams.
1. The command-line processor. the `-X` or `--shell` command-line switches channel the primary output stream to the command line processor for execution as commands.
1. A **field identifier**, which is a temporary buffer for use within the same iteration of the specification and in statistics.

For more information about secondary input and output streams, see the [relevant sections in chapter 12](#multstrm)

This makes **specs** a natural glue between other Unix tools, a role similar to that of **sed**, **awk**, or **cut**.

## Three Places to Give Instructions

You can give **specs** its specification in two places: either as arguments on the command line, or as a **spec file** using the `-f filename` or `--specFile filename` command line switches. Specifying on the command line is appropriate for short, one-off tasks or as part of a larger script. Specifying in a file is appropriate for longer specifications, used more than once, with comments and indentation.  Additional information is given to specs in the configuration file -- `/home/johnsmith/.specs` on POSIX systems, or `C:\Users\johnsmith\specs.cfg` on Windows -- values that stay the same across invocations: timezone, locale, personal constants, etc.  

You will learn all three. A quick decision guide is in Appendix A; details are in Chapters 3, 4, and 5.

## A Note About Examples

As stated above, you can run **specs** with a *specification* on the command line, or place the *specification* in a file. With a file, you get indentation and comments for better readability. For the readability of this guidebook, some of the examples are given as command line, some as a file, and some with a kind of mixture that is not possible in real usage. See the following example copied from [Chapter 12](#chap12):
```
specs --inCmd "ls -l"
    /Filename:/ 1
    W-1         NW   # Last word in "ls -l" output is file name
    WRITE            # Skip to next line
    /type:/ 1
    IF "range(1,1)=='d'" THEN
        /directory/ nw
    ELSE             # if we wanted to be fancy, we'd add an ELSEIF for 'l' and /link/
        /file/ nw
    ENDIF
```

We have the **specs** command, followed by the command flag and the `ls -l` command and no `--specFile` switch, so this *specification* is given as arguments on the command line. And yet we show it as multi-line and with indentation and comments. This is done for readability.

---

# Chapter 2: Basic Specifications - Data Fields

## Running specs

The basic invocation is:

```
specs [switches] [spec-units]
```

Switches begin with `--` (or `-` for single-letter forms) and must come before spec units. All remaining arguments form the specification.

The very simplest specification is a *pass-through*: copy the entire input record to position 1 of the output:

```
echo "Hello, world" | specs 1-* 1
```

Output:
```
Hello, world
```

`1-*` means "characters 1 to the end of the record." `1` means "place the result at output column 1." That is a complete, minimal spec.

## Spec Units and Data Fields {#datafield}

A **data field** is the workhorse spec unit. Its full form is:

```
[fieldIdentifier:] InputSource [STRIP] [conversion] OutputPlacement [alignment]
```

Here is what each part does, in the order it appears on the command line:

- **fieldIdentifier** — a single letter followed by a colon (e.g. `a:`), used to save the value under a name for later reuse.
- **InputSource** — what to read from the input record: a character range, a range of words, a string literal, or one of several other sources. This and `OutputPlacement` are the only parts that are always required.
- **STRIP** — removes leading and trailing whitespace from the value before it is converted or placed.
- **conversion** — transforms the value, e.g. changing its case.
- **OutputPlacement** — where the (possibly converted) value goes: an absolute output column, or a column relative to the previous output.
- **alignment** — how to justify the value (`left`, `right`, or `center`/`centre`) when it is narrower than its output field.

Most parts are optional. At minimum you need an `InputSource` and an `OutputPlacement`. The rest of this section walks through each part in turn, covering enough for you to write specifications made up entirely of data fields. [Chapter 6 (input sources and conversions)](#chap6) and [Chapter 7 (output placement)](#chap7) go into full depth on each part. When multiple *data fields* are included in a *specification*, they are processed in order.

**Example:** - A specification with one **Data Field** Spec Unit that has only an **InputSource** and an **OutputPlacement**: 

```
echo "Hello, world" | specs 1-5 1
```

Output:
```
Hello
```

`1-5` selects characters 1 through 5; `1` places them at column 1 of the output.

### Selecting Characters and Character Ranges

Character ranges use 1-based indexing, with negative indices counting back from the end:

| Syntax | Meaning |
|--------|---------|
| `n` | Single character at position n |
| `m:n` | Characters m through n inclusive (wraps if `n<m`) |
| `1-*` | The entire record |

**Note**: You can use `m-n` instead of `m:n`, but this older form has some limitations. It doesn't work if `n` is smaller than `m`, or if `n` is negative. The range is treated as a **[string literal](#sliteral2)** and printed verbatim, so `specs 5-3 1` outputs the text `5-3`.

Let's try a few:

```
echo ABCDEFGH | specs 3-5 1
```
Output: `CDE`

```
echo ABCDEFGH | specs 1:-3 1
```
Output: `ABCDEF` (from position 1 to the 3rd-from-last character)

```
echo ABCDEFGH | specs 5:3 1
```
Output: `EFGHABC`

**[Chapter 6](#chap6)** covers the full set of range forms.

### Selecting Words and Word Ranges

A **word** is a sequence of non-whitespace characters. Words are separated by one or more whitespace characters (the default word separator).

| Syntax | Meaning |
|--------|---------|
| `w1` or `WORD 1` | First word |
| `w2:4` or `WORD 2:4` | Words 2 through 4 |
| `w-1` or `WORD -1` | Last word |

**Note**: Similar to character ranges, you can use `w2-4` here as well, with the same limitations.

```
echo "the quick brown fox" | specs w2 1
```
Output: `quick`

```
echo "the quick brown fox" | specs w1:2 1
```
Output: `the quick`

```
echo "the quick brown fox" | specs w-1 1
```
Output: `fox`

The whitespace separator can be set with the `WORDSEPARATOR` or `WS` keywords. **specs** also has **fields**. Unlike words, fields are separated by a single *field separator*, which by default is a `tab`. More on this in **[Chapter 6](#chap6)**.

### String Literals as InputSource {#sliteral2}

You can place literal text in the output by using a string literal as the input source:

```
specs "Hello, there" 1
```
Output: `Hello, there`

Delimiters can be used when necessary to avoid confusion:

```
specs /word/ 1
```
Output: `word`

On the command line, double quotes are a signal to the **shell** rather than to **specs**. They tell the shell that everything enclosed — spaces, special characters and all — is a single argument. **specs** then treats that whole argument as a *string literal*, unless the entire thing looks like something else: a keyword, a token, a range, and so on. So a command-line literal usually needs no delimiters of its own.

In a *spec file* (see **[Chapter 5](#chap5)**) there is no shell to group words into arguments for you, so a string literal that contains spaces **must** be surrounded by a delimiter. A double quote can serve as that delimiter, but slashes are the most common:

```
/Hello, there/  1
```

### Field Identifiers

A **fieldIdentifier** is a single letter followed by a colon, such as `a:`, placed at the start of a data field. It saves the value under that name so it can be reused elsewhere in the specification, either in an *expression* (see **[Chapter 8](#chap8)**), or with the `ID` keyword:

```
echo "5 3" | specs a: w1 . b: w2 . ID b 1 ID a 2
```
Output: `35`

Field identifiers, and everything you can do with them, are covered in full starting in Chapter 6.

### STRIP — Trimming Whitespace

Adding the `STRIP` keyword right after the `InputSource` removes leading and trailing whitespace from the value before it is converted or placed:

```
echo "  hello  " | specs "<" 1 1-* strip NEXT ">" NEXT
```
Output: `<hello>`

This is handy when a fixed-width or word-delimited source includes surrounding blanks that you don't want to carry into the output.

### Conversions

A **conversion**, placed just before the `OutputPlacement`, transforms the value. `UCASE` and `LCASE` are the two most common examples — there are others, all covered in [Chapter 6](#chap6).
\newpage
Example:
```
echo "Hello World" | specs 1-* ucase 1
```
Output: `HELLO WORLD`

### Output Placement

The `OutputPlacement` argument says where the value goes. You've already seen the simplest form — an absolute column number, such as the `1` in `1-5 1`. You can also place output *relative* to whatever was written previously, which avoids having to compute absolute column numbers by hand. Use `N` or `NEXT` to place the output immediately after the previous output. Use `NW` or `NEXTWORD` to place it after a space following the previous output.

```
echo "Alice 42" | specs /Name:/ 1 w1 NEXTWORD /Age:/ NEXTWORD w2 NEXTWORD
```
Output: `Name: Alice Age: 42`

Note that `NEXTWORD` adds a single space before the next piece of output, but *will not* do so for the first *data field* in the specification. When OutputPlacement can be omitted (for example, at the end of a specification), the default is `NEXTWORD`.

Other output placement options are covered in Chapters 6 and 7.

### Alignment

The optional `alignment` argument comes last, and only makes sense when the output field is wider than the value being placed (for example, when `OutputPlacement` is a range like `1-10`). It can be `left` (the default), `right`, or `center`/`centre`:

```
echo "42" | specs "#" 1 1-* 2-10 right "#"
```
Output: `#       42 #` - also an example of an elided final OutputPlacement.

### A Practical Example

Suppose you have a log file where each line looks like:

```
2024-01-15 14:32:07 ERROR Connection refused from 192.168.1.100
```

And you want to extract just the date, severity, and IP address:

```
cat logfile.txt | specs w1 1 w3 13 w-1 25
```

Output:
```
2024-01-15  ERROR       192.168.1.100
```

---

# Chapter 3: When to Use Command-Line Flags

Command-line flags (switches) modify how specs behaves. They come *before* the spec units on the command line. Most simple invocations don't need any flags — the defaults cover the common case.

Here is a guided tour of when each flag is useful.

## Input and Output

### `-i filename` / `--inFile filename`

Read input from a file instead of stdin. Use this when you're not piping from another command:

```
specs -i mydata.txt 1-* 1
```

### `-o filename` / `--outFile filename`

Write output to a file instead of stdout. Useful when the output is large or when you want to avoid it appearing on screen:

```
cat input.txt | specs -o output.txt w2 1
```

### `-C cmd` / `--inCmd cmd`

Use the output of a shell command as the input stream:

```
specs --inCmd "ls -l /tmp" w9 1
```

This avoids a pipe when you want the input to be the output of a command.

### `--is2` through `--is8` / `--os2` through `--os8`

Assign files to additional input or output streams. This is an advanced feature covered in **[Chapter 12](#chap12)**.

### `--recfm format`, `--lrecl n`, `--linedel char`

Control how input records are structured. The default (`D` for delimited) reads one line at a time. Use `F` for fixed-length records (requires `--lrecl`) or `FD` for fixed-length delimited lines. For details, see the **[Record Formats](#recfm)** section of Chapter 6; a quick reference table is in **[Appendix B](#appendixb)**.

## Controlling Output

### `-X` / `--shell`

Execute each output record as a shell command rather than printing it. This is a powerful but dangerous flag — make sure your specification is correct before using it! A typical use is generating a series of shell commands:

```
ls *.log | specs --shell /rm/ 1 w1 nextword
```

This would delete all `.log` files. Use without `--shell` first to verify the commands look right.

### `--toASCII`

Convert non-ASCII characters in the output to periods. Useful when downstream tools can't handle Unicode.

### `--progress`

Print a counter to stderr every second showing how many records have been processed. Useful for long-running jobs on large files.

## Debugging and Diagnostics

### `-v` / `--verbose`

Print extra information when something goes wrong. When rolling context is in use (**[Chapter 13](#chap13)**), it also reports the buffer sizes:

```
specs: Using a 3-record rolling context: 2 records forward and 1 records backward.
```

Use `-v` as your first step when a specification produces unexpected output.

### `--stats`

Print runtime statistics to **standard error** at the end of the run: record counts, wall-clock time, and CPU time. Useful for performance tuning:

```
$ specs --stats -i large_file.txt -o /dev/null 1-* 1

Read  36160 lines.
Wrote 36160 lines.
Run Time: 0.030199 seconds.
CPU Time: 0.529951 seconds.
Main Thread:
        Initializing: 148.513 us (0.490%)
        Processing: 16.252 ms (53.575%)
        Waiting on IO: 13.926 ms (45.909%)
        Draining: 7.820 us (0.026%)
```
\newpage
## Behavior Modifiers

### `-f filename` / `--specFile filename`

Read the specification from the named file instead of the command line. This is covered in **[Chapter 5](#chap5)**.

### `-c filename` / `--config filename`

Use a different configuration file instead of the default one — `~/.specs` on POSIX systems (Linux and macOS), or `specs.cfg` in your home directory on Windows.

This is useful for testing, and for keeping a separate set of configured literals per project:

```
specs -c ~/specs-project1 -f myspec.txt
```

It is even more useful for sharing one configuration file among several users. A configuration file in a shared location gives an entire team the same configured literals, timezone, and locale, so that everybody's specifications produce identical results — and a correction needs to be made in only one place:

```
specs -c /etc/specs/team.cfg -f myspec.txt
```

### `-s name=value` / `--set name=value`

Set a configured literal (see **[Chapter 4](#chap4)**) from the command line. This overrides any value from the configuration file, and lets you parameterize a specification without editing it:

```
$ cat sales.txt
widgets 150
gadgets 80
gizmos 220

$ specs -i sales.txt -s threshold=100 WORD 2 a: IF "a > @threshold" THEN w1 1 w2 nw ENDIF
widgets 150
gizmos 220

$ specs -i sales.txt -s threshold=200 WORD 2 a: IF "a > @threshold" THEN w1 1 w2 nw ENDIF
gizmos 220
```

### `-t` / `--threaded`

Run in threaded mode: separate threads for the reader, the processor, and the writer. This can improve throughput on large files when *processing* is the bottleneck, because the processing thread no longer has to block while records are read and written. If the I/O itself is the bottleneck, threaded mode will not help — there is nothing for the processor to get on with while it waits. The default since version 0.9.5 is single-threaded.

**Deprecated** — threaded mode may be removed in a future release. Most practical specifications turn out to be I/O-bound rather than CPU-bound, so there is rarely anything for threaded mode to gain. Avoid relying on it in new work.

### `--spaceWS` / `-w`

Treat only the space character as a word separator, rather than all locale-defined whitespace. By default, tabs and other whitespace characters also separate words.

### `--no-while-guard`

Disable the while-guard, which normally causes specs to abort after 5000 iterations of a `WHILE` loop (a safety net against infinite loops). Use this when you intentionally have a long-running loop. The default limit of 5000 can also be changed via the `while-guard-limit` key in the configuration file (**[Chapter 4](#chap4)**).

### `--EXP-UTF8`

**Experimental** — enable UTF-8 aware character counting. When this flag is set, multi-byte UTF-8 sequences are treated as a single logical character for the purposes of character-range selection, word start/end positions, and output column calculations. This flag is marked experimental because it is not fully tested across all features.

### `--debug-alu-comp`

*(Debug builds only.)* Print detailed information about the parsing and compilation of ALU expressions. Useful for diagnosing why an expression is being parsed differently than expected.

### `--debug-alu-run`

*(Debug builds only.)* Print step-by-step evaluation information for ALU expressions at runtime. Very verbose; useful for tracing a complex expression's intermediate values.

### `--timezone name`

Convert to and from time-formatted strings using the named timezone. Values come from the TZ database, such as `America/New_York`, `Europe/London`, or `Asia/Tokyo`. See [Wikipedia](https://en.wikipedia.org/wiki/List_of_tz_database_time_zones) for the full list. This can also be set in the configuration file.

### `--regexType optionList` {#regextype}

A few of the built-in ALU functions — `rmatch()`, `rsearch()`, and `rreplace()`, all described in **[Chapter 9](#chap9)** — match strings against *regular expressions*. This switch sets the grammar in which those regular expressions are written.

The default is `ECMAScript`. Other options include `basic`, `extended`, `awk`, `grep`, and `egrep`. You can also combine flags like `icase` for case-insensitive matching. This can also be set in the configuration file.

## Python Functions

### `--pythonFuncs on/off/auto`

Control loading of Python functions (see **[Chapter 14](#chap14)**). The default, `auto`, loads Python only when an unknown function name is encountered. Set to `off` to disable Python entirely. Set to `on` to always load Python even when no unknown functions appear.

If your build of specs was compiled without Python support, this flag makes no difference — Python functions are never loaded in any case.

### `--pythonErr throw/NaN/zero/nullstr`

Determine what happens when a Python function raises an exception. The default, `throw`, causes specs to abort with an error message. Alternatives: `NaN`, `zero`, and `nullstr` return a safe value instead of aborting.

## Help and Information

### `--help topic`

Print help for a topic without running specs. Topics include: `help` (how to use this switch), `pyfuncs` (Python functions), `builtin` (built-in functions), `specs` (the specifications on the `SPECSPATH`), or the name of a specific function or specification. The help text for a specification comes from the comments at the top of its spec file, as described in **[Chapter 5](#docblock)**.

### `--info`

Print build information: version, platform, Python version, compiler, build source, commit hash, and build time. Example output:

```
specs invoked as 'specs'
        Compiler version: 9.2.1 20191120 (Red Hat 9.2.1-2)
        High/low watermark for queues: 5000 / 4500
        Random Provider: rand48
        Git tag: dev-1.0.0
        Python version: 3.12.1
        Floating point precision: 18 (16 bytes)
```

### `--force-read-input`

Force specs to read input records even if no spec unit references input. By default, if no spec unit reads from the input (e.g., `specs @version 1`), specs does not bother reading stdin. Use this flag to still consume and count records.

## Summary: When to Add a Flag

| Situation | Flag to use |
|-----------|-------------|
| Reading from a file | `-i filename` |
| Writing to a file | `-o filename` |
| Executing output as shell commands | `-X` |
| Spec is too long for the command line | `-f filename` |
| Something's going wrong and you need more info | `-v` |
| Profiling performance | `--stats` |
| Working with a different timezone | `--timezone name` |
| Disabling or forcing Python | `--pythonFuncs off/on` |

---

# Chapter 4: The Configuration File {#chap4}

The **configuration file** is a plain-text file that defines *configured literals* — named constants that specs can use in specifications. It also contains settings that affect specs's default behavior.

## Location

- **Linux and macOS**: `~/.specs` (in your home directory)
- **Windows**: `%HOME%\specs.cfg`

You can override this with the `--config` flag (Chapter 3).

## Format

The file contains `name: value` pairs, one per line:

```
pi: 3.14159265
favoriteAnimal: cat
billion: 1000000000
timezone: Asia/Bangkok
locale: en_US
Motto: "memento mori"
```

- Names are case-sensitive.
- Values can be numeric or string.
- String values that contain spaces should be enclosed in double quotes.
- Lines beginning with `#` followed by a space are comments.

## Using Configured Literals in Specs

Configured literals are referenced with a leading `@` in expressions. For example, with `pi: 3.14159265` in `~/.specs`:

```
specs PRINT "@pi*2" 1
```

Output: `6.28318530`

Outside of expressions, `@name` is expanded directly. In the following data field:
```
specs @pi 1
```
...`@pi` is expanded to `3.14159265` before specs sees it, so this becomes equivalent to:
```
specs 3.14159265 1
```
which is a literal (treated as a string `3.14159265`).

Inside expressions, `@pi` is the numeric value of pi and participates in arithmetic:
```
specs PRINT "@pi * @pi" 1
```
Output: `9.86960440108936`

## Special configured literals that act as settings {#configured-literal-settings}

### `timezone`

Sets the timezone for date/time conversion functions:
```
timezone: America/New_York
```
This is equivalent to `--timezone America/New_York` on the command line, but applies to every invocation.

### `locale`

Sets the locale for number formatting functions like `pretty()`:
```
locale: en_US
```
The special value `global` resets this to the process's global (OS-provided) locale instead of a specific one:
```
locale: global
```

### `regexType`

Sets the default regular expression grammar and flags used by `rmatch()`, `rsearch()`, and `rreplace()`, equivalent to `--regexType` on the command line but applied to every invocation:
```
regexType: extended,icase
```
See [`--regexType`](#regextype) in Chapter 3 for the list of valid options.

### `SPECSPATH`

A colon-separated list of directories where specs looks for spec files ([Chapter 5](#chap5)) and Python functions ([Chapter 14](#chap14)):
```
SPECSPATH: /home/alice/specs:/usr/local/share/specs
```
**Note:** On Microsoft Windows the directories are separated by semicolons.

If not set, `SPECSPATH` defaults to `$HOME/specs` on POSIX systems and to `%APPDATA%\specs` on Windows.

### `pythonDisable`

Set to `1` to permanently disable Python function loading. Unlike `--pythonFuncs off`, this cannot be overridden from the command line:
```
pythonDisable: 1
```

### `NO_WARN_REDEFINED_FID`

Set to any value to suppress the warning that specs emits when a field identifier is re-defined within the same spec. Setting this would eliminate the warning below:
```
$ echo "hello there" | specs a: WORD 1 1 a: WORD 2 NEXTWORD
WARNING: Field Identifier <a> redefined.
hello there
```

### `EmptyFrequencyMapMessage`

`fmap_dump()` is one of the frequency map functions described under **[Statistical and Frequency Map Functions](#statistical-and-frequency-map-functions)** in **[Chapter 9](#chap9)**. It returns a printable summary of the values accumulated in a field identifier.

This key sets the string that `fmap_dump()` returns when the frequency map contains no data at all. The default is an empty string. So for example, if the file contains this entry:
```
EmptyFrequencyMapMessage: "(no data)"
```
Then you get the following:
```
$ echo "hello" | specs WORD 1 1 PRINT "fmap_dump(a)" NEXTWORD
hello (no data)
```

### `while-guard-limit`

Changes the maximum number of iterations allowed in a `WHILE` loop before specs aborts with a "potentially endless loop" error. The default is **5000**. Set to a higher value when a spec legitimately needs more iterations:

```
while-guard-limit: 100000
```

This is equivalent to using `--no-while-guard` if you set it high enough, but safer because it still caps runaway loops. Use `--no-while-guard` only when you truly need unlimited iterations.

## Useful Personal Constants

The configuration file is a good place to put any value you use in multiple specifications:

```
# Conversion factors
mph_to_kph: 1.60934
lbs_to_kg: 0.453592

# Project-specific
projectRoot: /home/alice/myproject
reportHeader: "Monthly Report for"
```

## Pre-defined Configured Literals

These exist automatically without any `~/.specs` entry:

| Name | Value |
|------|-------|
| `@version` | The specs version string, e.g., `1.0.0` |
| `@platform` | Platform description (OS, compiler, Python version) |
| `@cols` | Number of columns in the terminal |
| `@rows` | Number of rows in the terminal |
| `@python` | Either `Enabled` or `Disabled` |
| `@build-commit` | Short git commit hash of the build |
| `@build-branch` | Git branch name |
| `@build-time` | Build timestamp (ISO 8601) |
| `@build-source` | `local` or `github` |
| `@build-number` | GitHub Actions build number (empty for local) |
| `@build-runid` | GitHub Actions run ID (empty for local) |
| `@build-url` | GitHub Actions build URL (empty for local) |
| `@build-info` | Composite build information string |
| `@@` | The entire current input record (in expressions) |
| `@!` | The current in-context input record (see `CONTEXT` in [Chapter 13](#chap13)) |
| `@+n` / `@-n` | Record at offset +n / -n from current (see [Chapter 13](#chap13)) |

## Ensuring a Literal is Defined — REQUIRES

If your specification depends on a configured literal, you can protect it with `REQUIRES`:

```
$ echo -e "1\n2\n3" | specs REQUIRES pi PRINT "2*@pi*word(1)"
6.28318530717958648
12.566370614359173
18.8495559215387594
```

If `pi` is not defined in `~/.specs` or via the `--set` switch on the command line, specs will abort with a clear error message while parsing the specification:
```
Error while parsing command-line arguments:
Missing required configured literal <pi>
```

---

# Chapter 5: Specification Files {#chap5}

When a specification grows beyond a few data fields, putting it all on the command line becomes unwieldy. **Specification Files** (or just **Spec files**) let you write specifications in a file with comments, indentation, and blank lines.

## Using a Spec File

```
specs -f myspec.txt
```

or equivalently:

```
specs --specFile myspec.txt
```

A spec file replaces the command-line specification entirely, so the two cannot be combined. If you supply both a spec file and spec units, specs rejects the invocation rather than silently ignoring one of them:

```
$ specs -f myspec.txt -i input.txt w1 1
A spec file (--specFile) cannot be combined with spec units on the command line: <w1>
```

The specification is the only thing that moves into the file. Input and output are still supplied in the usual way — from standard input and standard output, or with `-i` and `-o`:

```
specs -f myspec.txt -i input.txt -o report.txt
```

## Where Spec Files Go {#the-specspath}

You can place a spec file anywhere. Just give an absolute path, and **specs** will find it:
```
$ specs -f /home/alice/specs/myspec
```

It's usually better to place the specifications in specific, dedicated directories and then only tell **specs** the name of the specification (`myspec` in the example). When you specify a relative filename with `-f`, specs searches for it in the **SPECSPATH** — a colon-separated list of directories. The SPECSPATH is controlled by:

1. The `SPECSPATH` environment variable
2. The `SPECSPATH` entry in `~/.specs`
3. The default: `$HOME/specs` on Linux/macOS, `%APPDATA%\specs` on Windows

So if your `SPECSPATH` is `/home/alice/specs`, you can store your spec files there and reference them as `-f myspec` without specifying a full path.

\newpage

## Format

A spec file is a plain text file. Spec units are written as if they were command-line arguments, but you can spread them across multiple lines and add comments:

```
# This counts and adds up a list of numbers
    printonly eof
    a: word 1
       set "#0+=a"     # Counter #0 is an accumulator
       set "#1+=1"     # Counter #1 is a counter
    eof
       /Total:/        1
       print #0 strip  nextword
       /in/            nextword
       print #1 strip  nextword
       /records./      nextword
```
**NOTE:** `printonly` is explained in [Chapter 11](#chap11).

## Comments

There are two styles of comments in spec files:

1. **Full-line comment**: The line begins with "`# `" (hash+space), optionally preceded by whitespace. The entire line is ignored.
2. **End-of-line comment**: The comment begins at the last occurrence of "` # `" (space+hash+space) that is also preceded by whitespace, or at a trailing hash mark that is the very last character on the line. Everything from that hash mark onward is ignored.

**Example:**
```
# Example specification with a comment on every line
WORD 1  1          # This puts the first word at column 1
/hello/ NEXTWORD   # This appends the word "hello"
```

Note: The hash+space ("`# `") must be preceded by whitespace for an end-of-line comment. A hash at the last position in the line can be a comment even though it is not followed by a space, but the comment is empty. A hash inside a literal string (`/hello # world/`) is not a comment.

## Documenting a Spec File {#docblock}

Comments at the very top of a spec file do double duty: besides documenting the file for anyone reading it, they become the file's **help text**. This lets you find out what a spec file does without opening it — useful once you have accumulated a directory of them on the **[SPECSPATH](#the-specspath)**.

Consider a spec file named `wordfreq`:

```
# Count word frequencies in the input.
# Reads one word per record and prints a
# frequency table at end of file.
#
# Usage: specs -f wordfreq -i words.txt
printonly eof
   a: word 1
eof
   print "fmap_dump(a)" 1
```

There are two ways to ask for this help text, and they deliberately show different amounts of it.

### Listing all specifications

`specs --help specs` lists every spec file on the `SPECSPATH`, each with a one-line summary taken from **only the first line** of the file:

```
$ specs --help specs
Specification <wordfreq> -  Count word frequencies in the input.
Specification <nodoc>
Specification <plainspec>
```

Because only the first line appears here, make it a self-contained summary. A file whose first line is not a comment is still listed, just without a description — as with `nodoc` and `plainspec` above.

### Help for one specification

Naming a single specification shows the **whole** opening comment block:

```
$ specs --help wordfreq
Specification <wordfreq>
	 Count word frequencies in the input.
	 Reads one word per record and prints a
	 frequency table at end of file.

	 Usage: specs -f wordfreq -i words.txt
```

So the first line belongs in the summary, and the remaining lines are the place for usage notes, the expected input format, and examples.

### Rules for the help text

The opening comment block obeys stricter rules than comments elsewhere in the file:

- It must begin on the **first line** of the file, and it ends at the first line that does not start with a hash mark. Comments further down the file are ordinary comments and never appear in help output.
- The hash mark must be in **column 1**. An indented comment is a perfectly good comment, but it is not help text.
- Write the hash mark followed by a space. A hash mark with nothing after it is fine and produces a blank line in the help text, but `#likethis` is *not* a comment as far as the specification parser is concerned — it appears in the help text and then makes the specification fail to compile.

Files whose names begin with a period or an underscore, and files whose names contain `.py`, are not treated as specifications and are not listed.

## Directives

Directives are special instructions that must appear at the **very beginning** of the spec file, before any spec units. Every directive line must begin with a `+` character in the first column.

### `+SET name command`

Runs a shell command and stores its first line of output as a configured literal named `name`:

```
+SET mydate date +%Y-%m-%d
+SET hostname hostname
```

After this, `@mydate` and `@hostname` are available in the specification.

### `+IN command`

Runs a shell command and uses its output as the primary input stream. This replaces stdin:

```
+IN ls -l /var/log
```

Combined with a spec, this makes a self-contained script:

```
+IN ls -l /var/log
w9 1 w5 nw
```

## The `REQUIRES` Keyword

Use `REQUIRES` at the start of a spec (before any spec units but after directives) to declare that a configured literal must be defined. If the literal is absent, specs aborts immediately with a helpful error:

```
REQUIRES threshold
REQUIRES timezone

w1 1
IF "w1 > @threshold" THEN
    /EXCEEDS LIMIT/ nextword
ENDIF
```

## When to Use a Spec File Instead of the Command Line

Use a spec file when:

- The specification has more than 3–4 data fields or spec units.
- You need comments to explain what the specification does.
- You want to reuse the specification in multiple places.
- You need directives (`+SET`, `+IN`).
- You want to use `REQUIRES` to validate dependencies.

Use the command line when:

- The specification is a quick one-off.
- You're exploring or prototyping.
- The specification fits comfortably in one or two terminal lines.

---

# Chapter 6: Selecting and Transforming Input {#chap6}

Every data field begins with an **input source** — a description of where to get the data for this field. This chapter covers all input source types and the conversions that can be applied to them.

## Character Ranges

The simplest input source is a range of character positions (1-based):

| Syntax | Meaning |
|--------|---------|
| `n` | Single character at position n |
| `n:m` | Characters n through m inclusive |
| `n.len` | `len` characters starting at position n |
| `n-m` or `n;m` | Alternate separators for `n:m`, each with a caveat (see below) |
| `-n` | n-th character from the end (`-1` is the last) |
| `1-*` | The entire record |

The colon (`:`) is the recommended separator. The other two both work, but neither is a drop-in replacement for it.

The semicolon (`;`) is included for **CMS Pipelines** compatibility. The shell uses a semicolon to separate commands, so a range written with one has to be quoted — `specs '2;-2' 1` rather than `specs 2:-2 1`.

The hyphen (`-`) reads naturally but you cannot have a negative value in the last position. `specs 1--3` is interpreted as the **string literal** "`1--3`", and the range `1:-3` (from position 1 to 3 before the end) has no hyphenated equivalent.

When the first position of a range is greater than the second, the selection **wraps around** the end of the record. On the input `abcdefgh`, `5:2` yields positions 5 through 8 followed by positions 1 and 2:

```
echo "abcdefgh" | specs 5:2 1
```
Output: `efghab`

Wrap-around is available only through the colon and semicolon forms; `5-2` is a string literal.

## Words

Words are separated by the **word separator**, which defaults to any locale-defined whitespace character.

| Syntax | Meaning |
|--------|---------|
| `w1` or `word 1` | First word |
| `w-1` or `word -1` | Last word |
| `w2:4` or `word 2:4` | Words 2 through 4 |
| `w2-4` or `w2;4` | Alternate separators for `w2:4`, with the same caveats as character ranges |
| `w2.3` | 3 words starting at word 2 |

The keyword may be abbreviated to any prefix, so `word 2:4`, `wor 2:4`, `wo 2:4` and `w 2:4` are all accepted. The entire input, from the start of the first word to the end of the last specified word (including any whitespace in between), is captured as the value. Note that the whitespace *around* the selection is not included:

```
echo "  hello   world   foo   bar  " | specs w2:3 1
```
Output: `"world   foo"`

## Fields

Fields are separated by the **field separator**, which defaults to a tab character. Unlike words, the separator between fields is exactly one character, so empty fields are possible.

| Syntax | Meaning |
|--------|---------|
| `f1` or `field 1` | First field |
| `f-1` or `field -1` | Last field |
| `f2:4` or `field 2:4` | Fields 2 through 4 |
| `f2-4` or `f2;4` | Alternate separators for `f2:4`, with the same caveats as character ranges |
| `f2.3` | 3 fields starting at field 2 |

As with words, the keyword `FIELD` may be abbreviated to any prefix and used in any capitalization. On the tab-separated input `a\t\tb`, field 1 is `a`, field 2 is empty, and field 3 is `b`.

## Words vs. Fields: The Key Distinction

- **Words**: Separated by *one or more* separator characters. Multiple consecutive separators count as one. Empty words are impossible.
- **Fields**: Separated by *exactly one* separator character. Two consecutive separators produce an empty field between them.

Example with comma as both word and field separator on `hello,,,,there`:

- Word 1 = `hello`, word 2 = `there` (the commas collapse)
- Field 1 = `hello`, fields 2–4 = `""` (empty), field 5 = `there`

## Changing the Separator

### WORDSEPARATOR (WS)

Use at the beginning of a spec to change what counts as a word separator:

```
echo "/Good///bye/old///paint" | specs wordseparator / word 2 1
```
Output: `bye`

You can specify multiple word separator characters as a string:
```
specs ws ",:;" word 1 1
```
This treats commas, colons, and semicolons all as word separators.

The special value `default` resets to locale-defined whitespace.

### FIELDSEPARATOR (FS)

Similar to `WORDSEPARATOR`, but changes the field separator:

```
echo "Good,bye,old,,paint" | specs fieldseparator , /</ 1 f2 n />/ n /</ nw f4 n />/ n
```
Output: `<bye><>`

Both `WORDSEPARATOR` and `FIELDSEPARATOR` are **MainOptions** — they apply to the entire specification and should appear before any data fields. 
Practically, field separators are more often overridden than word separators. One common usecase is using a comma or semicolon to parse a comma-separated values (CSV) file. Another is using the slash character (`/`) as field separator to break a full Unix path into directories.

## The SUBSTRING Input Source

For a more complex selection out of the input record, use `SUBSTRING` (or just `SUBSTR`):

```
SUBSTRing [WORDSEP char] [FIELDSEP char] range OF InputSource
```

This is quite verbose, but it is still just an `InputSource`, and it needs an `OutputPlacement` after it just like any other. What makes it unusual is that it is an `InputSource` that *contains* another `InputSource`: it selects a range (character, word, or field) from *within* whatever that inner source produces. The `WORDSEP` and `FIELDSEP` options let you set different separators just for this substring selection, independent of the global settings.

**Example**: extract the bare filename from a list of paths. `find` prints one path per line, so the whole path is word 1; splitting that word on slashes and taking the last field gives the final component:

```
find . -type f -name "*.md" | specs SUBSTR FS / FIELD -1 OF WORD 1  1
```

For the record `./specs/docs/guidebook.md`, this outputs `guidebook.md`.

## Special Input Sources

### `NUMBER` or `RECNO`

A 10-digit decimal record counter (padded with spaces on the left):

```
echo -e "a\nb\nc" | specs recno 1 w1 nw
```
Output:
```
         1 a
         2 b
         3 c
```

### `TODclock`

An integer number of microseconds since the Unix epoch — midnight of January 1st, 1970 — representing the time the current specs run started:

```
specs todclock 1
```
Output: `1785757989732438` (depending on when you run it)

Because its value is fixed for the whole run, `TODclock` does not force *specs* to read input records. A specification that contains no other input-reading source will therefore produce a single output record.

### `DTODclock`

Like `TODclock`, but gives the time of processing the **current** input record rather than the start of the run. Unlike `TODclock`, it *does* force the reading of input records. A specification that only has a data field with `DTODclock` as input produces one output record per input record, and none with no input.

### `TIMEDIFF`

A 12-character decimal number giving microseconds since the start of the run. Like `DTODclock`, it forces reading input records. Useful for timing:

```
echo -e "first\nsecond\nthird" | specs timediff 1
```
Output (just an example):
```
          34
          71
         118
```

### The `PRINT` / `?` Source

Evaluates an ALU expression and uses the result as input. Covered in **[Chapter 8](#chap8)**.
```
$ echo -e "1\n2\n3\n4" | specs PRINT "word(1)*3" 1
3
6
9
12
```

### The `ID` keyword

Uses the stored value of a **field identifier** as the input source. Field identifiers are covered below.
```
$ echo -e "1\n2\n3\n4" | specs WORD 1 a: ID a 1 PRINT "a*a" NEXTWORD
1 1
2 4
3 9
4 16
```

### String Literals

A string enclosed in delimiters is placed literally in the output:

```
echo test | specs /Hello/ 1 w1 nw /!/ n
```
Output: `Hello test!`

Supported delimiters: `/`, `'`, `"`, `|`, and others. The delimiter must not appear in the literal itself (or use a different delimiter).

### Hex Literals

A token beginning with `x` followed by an **even number** of hexadecimal digits is treated as a binary string literal. The hex pairs are decoded to their corresponding bytes. This is useful for embedding non-printable characters or binary sequences in the output.

| Syntax | Value |
|--------|-------|
| `x41` | The single byte `A` (0x41) |
| `x4142` | The two-byte string `AB` |
| `x0A` | A newline byte |
| `xDEADBEEF` | A 4-byte binary string |

```
specs x48454C4C4F 1
```
Output: `HELLO`

Hex literals can be combined with conversions like `C2X` to round-trip binary data, or used to inject separator characters that are hard to type on the command line.

As with `TODclock`, a specification built only from literals, string or hex, reads no input, so nothing needs to be piped in.

## Field Identifiers

A **field identifier** is a single letter (a–z, A–Z) followed by a colon, placed before the `InputSource`. It captures the input value in a named variable that can be reused later:

```
echo "5 3" | specs a: w1 . b: w2 . PRINT "a*b" 1
```
Output: `15`

Here `a:` captures word 1 into the variable `a`, and `b:` captures word 2 into `b`. Then `PRINT "a*b"` multiplies them.

A field identifier can also appear as the **OutputPlacement**, in which case it captures the output value:

```
echo "hello world" | specs w1 ucase a:
```

Here `a:` captures the uppercased first word into `a`. The `.` (period) as OutputPlacement means "no output, just save the value."

## Conversions

Between the `InputSource` and the `OutputPlacement`, you can apply a **conversion**:

```
InputSource conversion OutputPlacement
```

### Text Conversions

| Conversion | Effect |
|------------|--------|
| `ucase` | Convert to uppercase |
| `lcase` | Convert to lowercase |
| `rot13` | ROT-13 cipher |
| `BSWAP` | Reverse byte order |

```
echo "Hello World" | specs 1-* ucase 1
```
Output: `HELLO WORLD`

### Binary/Hex/Character Conversions

| Conversion | Effect | Example |
|------------|--------|---------|
| `C2B` | Characters to binary digits | `"AB"` → `"0100000101000010"` |
| `C2X` | Characters to hex | `"AB"` → `"4142"` |
| `B2C` | Binary digits to characters | `"0100000101000010"` → `"AB"` |
| `X2CH` | Hex to characters | `"4142"` → `"AB"` |
| `b2x` | Binary data to hex | |
| `D2X` | Decimal to hex | `"314159265"` → `"12b9b0a1"` |
| `X2D` | Hex to decimal | `"12b9b0a1"` → `"314159265"` |

### Time Conversions

Time conversion arguments follow the strftime convention, with the addition of `%xf` for fractional seconds (where x is the number of digits, 0–6).

| Conversion | Effect |
|------------|--------|
| `ti2f format` | Internal 8-byte microseconds-since-epoch to formatted string |
| `tf2i format` | Formatted string to internal 8-byte microseconds-since-epoch |
| `s2tf format` | Decimal seconds-since-epoch to formatted string |
| `tf2s format` | Formatted string to decimal seconds-since-epoch |
| `mcs2tf format` | Microseconds-since-epoch integer to formatted string |
| `tf2mcs format` | Formatted string to microseconds-since-epoch integer |

Example — convert a date string to seconds since epoch:

```
echo "Oct 30 09:46:22" | specs w1-3 tf2s "%b %d %H:%M:%S" 1
```
Output: `1793346382.000000`

### STRIP

Adding `STRIP` between the input source and the conversion (or output placement) removes leading and trailing whitespace from the value before placing it:

```
echo "  hello  " | specs "<" 1 1-* STRIP NEXT ">" NEXT
```
Output: `<hello>`

## Record Formats — When Records Are Not Lines {#recfm}

By default, specs treats each newline-terminated line of input as one record. This is the right choice for most text files and shell pipelines. But some data — especially binary files, packed logs, and files generated by mainframe or legacy systems — is organized differently. The `--recfm` flag (short for **record format**) lets you tell specs how to slice the byte stream into records.

The concept comes directly from mainframe operating systems (IBM z/OS, MVS, and their ancestors) where **record format** is an OS-level attribute of every file and data set, not just a convention. On those systems the OS itself knows whether a file is line-delimited, fixed-length, or variable-length — and programs read records rather than bytes. specs borrows this vocabulary so that the same kind of data can be processed the same way on any platform.

### `--recfm D` — Delimited (default)

```
specs ... --recfm D
```

Records are separated by a delimiter character. If `--linedel` is not specified the platform's native line-ending sequence is used (LF on Linux/macOS, CRLF on Windows). This is the default mode and what you get if you omit `--recfm` entirely.

You can supply a custom delimiter with `--linedel`:

```
# Records separated by a pipe character
specs --recfm D --linedel '|' -i data.pipe-separated w1 1
```

### `--recfm F` — Fixed Length

```
specs ... --recfm F --lrecl N
```

specs reads exactly **N** bytes from the input stream per record, regardless of what those bytes are. There are no separators; the record boundary is purely positional. This is the right choice for tightly packed binary data, certain instrument outputs, or mainframe flat files where each record occupies a known, constant number of bytes.

`--lrecl` (logical record length) is **required** with `--recfm F`.

```
# Each record is exactly 80 bytes — no newlines needed
specs --recfm F --lrecl 80 -i punched-card-image.bin 1-10 1 21-30 nw
```

Note that character positions are still 1-based from the start of each record, so column 1 is the first byte of each 80-byte chunk, column 80 is the last, and so on.

### `--recfm FD` — Fixed Length with Delimiter

```
specs ... --recfm FD --lrecl N
```

A hybrid: specs reads one delimited line at a time (using the OS line-ending or a custom `--linedel`), but then treats the result as a fixed-length field of exactly **N** characters. Lines longer than N are truncated; lines shorter than N are padded with spaces on the right. This is useful for data that has line endings for human readability but whose fields are always at fixed positions within those lines.

```
# Lines may vary in actual length but are logically 132 characters wide
specs --recfm FD --lrecl 132 -i report.txt 1-10 1 101-110 nw
```

### Summary Table

| `--recfm` | Nickname | `--lrecl` | `--linedel` | What a "record" is |
|-----------|----------|-----------|-------------|---------------------|
| `D` | delimited | not used | optional | One line (custom or OS delimiter) |
| `F` | fixed | required | not used | Exactly `--lrecl` bytes, no delimiter |
| `FD` | fixed-delimited | required | optional | One delimited line, normalized to `--lrecl` chars |

---

# Chapter 7: Placing Output {#chap7}

The **output placement** tells specs where to put the result of an input source in the output record.

## Absolute Placement

Specify a column number:

```
echo "hello world" | specs WORD 1 1 WORD 2 10
```
Output: `hello    world`

The output record is padded with spaces to reach column 10.

You can also specify a range to constrain the width:

```
echo "hello world" | specs WORD 1 1-5
```
Output: `hello` (5 characters starting at column 1)

If the value is shorter than the range, it is padded (left-aligned by default). If longer, it is truncated.

You can also use a **width suffix** (a dot followed by a number) to specify the width:

```
echo "hello world" | specs WORD 1 1.5
```
Output: `hello` (equivalent to `1-5`)

The width suffix works with both absolute and relative placement.

## Relative Placement

| Keyword | Meaning |
|---------|---------|
| `N` or `NEXT` | Append the value mmediately after the previous output, no gap |
| `NW` or `NEXTWORD` | Add a space, then append the value |
| `NF` or `NEXTFIELD` | Add a tab, then append the value |
| `.` (single dot) | No output — the value is captured but not placed (used with field identifiers) |

The abbreviated forms (`N`, `NW`, `NF`) also accept alternate spellings `NWORD` and `NFIELD`.

`NEXT` places output exactly where the previous output ended:

```
echo "AB" | specs [ 1  WORD 1 N  ] N
```
Output: `[AB]`

\newpage

`NEXTWORD` inserts a space:

```
echo "hello world" | specs W1 1 W2 NW
```
Output: `hello world`

**Note:** If used on the first output-producing *spec unit* in the specification, all three of `NEXT`, `NEXTWORD`, and `NEXTFIELD` place the output at the start of the line, leaving no space or tab.

### Width Suffix on Relative Placement

As mentioned above, the width suffix also works with relative placement. Any of `NEXT`, `NEXTWORD`, and `NEXTFIELD` can take an optional width suffix in the form `.n`, making the output a fixed-width output field at the relative position:

| Syntax | Meaning |
|--------|---------|
| `NW.10` or `NEXTWORD.10` | Next-word position, 10-character output field |
| `NF.8` or `NEXTFIELD.8` | Next-field position, 8-character output field |
| `N.5` or `NEXT.5` | Next position, 5-character output field |

Alignment applies after the output width (default LEFT-aligned; add `RIGHT` or `CENTER` to change it).

```
echo "hello world" | specs W1 NW.10 RIGHT  W2 NW.10 RIGHT
```
Output: `     hello     world`

### Field Identifiers as Output Placement

Instead of placing output in the output record, you can save it in a **field identifier** (a single letter followed by a colon) for later use in expressions or other data fields. Use the field identifier as the output placement:

```
echo "42" | specs W1 a: PRINT "a * 2" 1
```
Output: `84`

Here, `W1` is saved in field identifier `a`, then used in the expression `a * 2`.

Field identifiers are useful for:

- Saving intermediate values for reuse
- Building complex calculations step by step
- Avoiding redundant input source extraction
- Calculate statistics on some or all records.

## Alignment

After the output placement, you can specify an alignment for values shorter than the output field:

| Keyword | Effect |
|---------|--------|
| `LEFT` | Pad on the right (default) |
| `RIGHT` | Pad on the left |
| `CENTER` or `CENTRE` | Pad equally on both sides |

\newpage
```
echo "42" | specs "<" 1 1-* 2-10 RIGHT ">" NEXT
```
Output: `<       42>`

```
echo "hello" | specs "<" 1 1-* 2.20 CENTER ">" NEXT
```
Output: `<       hello        >`

**Note:** Alignment also applies when the value is *longer* than the output field. In this case, the value is truncated, and which part is removed depends on the alignment: `LEFT` removes the right end, `RIGHT` removes the left end, and `CENTER` removes from both ends.

```
echo -e "abcdefgh" | specs 1-8 1-4 LEFT
```
Output: `abcd`
```
echo -e "abcdefgh" | specs 1-8 1-4 RIGHT
```
Output: `efgh`
```
echo -e "abcdefgh" | specs 1-8 1-4 CENTER
```
Output: `cdef`

## The PAD Spec Unit — Changing the Padding Character

By default, gaps in the output record (spaces between placed fields, or padding added to make a value fit a larger output field) are filled with the **space character**. The `PAD` spec unit changes this fill character for all subsequent output in the same specification.

```
PAD /char/
```

The argument is a single character, using any delimiter. You can also write it without a delimiter if the character is unambiguous (e.g., `PAD =`).

`PAD` can appear multiple times to use different padding for different fields.

**Example — three different padding characters:**

```
echo "The quick brown" | specs PAD q W1 1.10 LEFT  PAD w W2 11.10 CENTER  PAD e W3 21.10 RIGHT
```
Output: `Theqqqqqqqwwquickwwweeeeebrown`

Breaking this down:

- `PAD q` sets padding to `q`; word 1 (`The`) placed LEFT-aligned in a 10-char field → `Theqqqqqqq`
- `PAD w` sets padding to `w`; word 2 (`quick`) placed CENTER-aligned in 10 chars → `wwquickwww`
- `PAD e` sets padding to `e`; word 3 (`brown`) placed RIGHT-aligned in 10 chars → `eeeeebrown`

**Example — fill the gap between two fields:**

```
echo "First record" | specs WORD 1 5  PAD =  WORD 2 15
```
Output: `    First=====record`

Here, the space from column 10 (end of "First") to column 14 (before "record") is filled with `=` instead of spaces.

**Important note about NEXTWORD and NEXTFIELD:** When using `NEXTWORD` or `NEXTFIELD` placement, the space or tab character inserted between fields is *not* replaced by the PAD character. It is always a literal space (for `NEXTWORD`) or tab (for `NEXTFIELD`). For example:

```
echo "hello there" | specs PAD = W1 1 W2 NW.9
```
Output: `hello there====`

The space between "hello" and "there" is a literal space (inserted by `NEXTWORD`), not a pad character. Only the padding added to fill the 9-character width is replaced with `=`.

`PAD` is a **MainOption** — it takes effect immediately where it appears and applies to all subsequent output until another `PAD` changes it again.

## Composed Output Placement

A **composed output placement** uses a parenthesized expression to compute the starting column, width, and alignment dynamically at runtime. It replaces both the regular output placement and alignment arguments:

```
InputSource (start_expr)
InputSource (start_expr, width_expr)
InputSource (start_expr, width_expr, align_expr)
```

The expressions are ALU expressions ([Chapter 8](#chap8)) evaluated each cycle.

Examples:

```
# Right-align a word in a 20-character field
specs w1 (20-length(word(1)))
```

```
# Dynamic triangles (position shifts with each record)
specs /##########/ (recno()%5 + 1, 2*(6 - recno()%5 - 1))
```

```
# Right-align in a 10-character field
echo "hello" | specs /hello/ (1,10,"R")
```
Output: `     hello`

### Alignment Strings in Composed Placement

The third expression is evaluated as a string:

- Begins with `c` or `C` → centered
- Begins with `r` or `R` → right-aligned
- Anything else → left-aligned

### Ellipsis Truncation

If the alignment string is exactly two characters and the second is a digit `1`–`5`, truncation uses an ellipsis when the value is longer than the field:

| Code | Behavior |
|------|----------|
| `?1` | Ellipsis then suffix: `...xyz` |
| `?5` | Prefix then ellipsis: `abcde...` |
| `?2` | 1/3 prefix, ellipsis, 2/3 suffix: `ab...yz` |
| `?3` | 1/2 prefix, ellipsis, 1/2 suffix: `abc...xyz` |
| `?4` | 2/3 prefix, ellipsis, 1/3 suffix: `abcd...z` |

```
echo "abcdefghijklmnopqrstuvwxyz" | specs 1-* (1,10,"R2")
```
Output: `ab...vwxyz`

### Eliding Arguments in Composed Output

You can omit leading arguments and use commas as placeholders. Omitting the start defaults to `next()`, and omitting the width defaults to `rest()`:

```
specs w1 (,,'R')   # next, full width, right-aligned
```

## Eliding The Entire Output Placement

Under certain conditions, it is possible to omit the **Output Placement** entirely. These are:

- On the last Data Field within a block
- On the one and only Data Field in the specification
- On the one and only Data Field in the run-out cycle

When the entire specification has just one data field that produces output, the **output placement** defaults to the first column - the start of the line. You can safely omit that:
```
specs W1   # Writes the first word of the input to the start of the output line
```

When the last Data Field within a block has no output placement, `specs` defaults to `NEXTWORD`. This is really a generalization of the previous case, because `NEXTWORD` on the first Data Field places the output at the start of the line. There is, however, a need to explain the meaning of "block" in this context.

A **block** refers to a portion of the specification that is executed or not executed at the same cycle. In the simplest case, a block is the whole specification:
```
specs WORD 1 1 WORD 2 20 WORD 3 
```

If you use **Control Flow** ([Chapter 10](#chap10)), you can get blocks before, in the middle, and after the parts of the control structure:
```
specs
   WORD 2  a:    # No output, just loading the word into A
   WROD 1  1
   WORD 3        # Last data field in the block, treated as NEXTWORD
   IF "0 = a%2" THEN   # if a is even
      /even/     # Elided placement at the end of the THEN block
   ELSE
      /odd/      # Elided placement at the end of the ELSE block
   ENDIF
   /number found/  # Elided placement at the end of the final block
```

For the input "foo 7 bar", you get the output `foo bar odd number found`

Similarly, just before the `EOF` keyword that marks the beginning of the run-out cycle ([Chapter 11](#chap11)), the last data field can skip the output placement:
```
echo "hello, there" | specs WROD 1 1 WORD 2 EOF /All done/
```
Output:
```
hello, there
All done
```

The one thing all these cases have in common is that the data field is followed by an obvious stop: either the end of the specification, or a keyword such as `IF`, `ELSE`, `ENDIF`, `EOF`.

---

# Chapter 8: Expressions and the ALU  {#chap8}

The **Arithmetic-Logical Unit (ALU)** is specs's computation engine. It evaluates *expressions* — combinations of values, operators, and function calls — to produce a result.

## Where Expressions Appear

Expressions appear in:

- `PRINT "expression"` — compute a value and place it in the output
- `SET "#n op expression"` — compute a value and store it in a counter
- `IF "condition"`, `WHILE "condition"` — control flow ([Chapter 10](#chap10))
- Composed output placement arguments ([Chapter 7](#chap7))

## The PRINT Input Source

`PRINT` is an **InputSource**, similar to the others described in [chapter 6](#chap6). It evaluates its argument as an ALU expression and places the result as the input value.

```
echo "" | specs PRINT "2+3" 1
```
Output: `5`

`PRINT` can be abbreviated as `?`:

```
specs ? "2+3" 1
specs "?2+3" 1    # the ? can be a prefix inside the argument
```

In bash, the standalone `?` form may trigger shell globbing, so the quoted `"?expr"` form is often safer.

## Elements of Expressions

### Numeric Literals

Standard integers and floating-point numbers:

```
specs PRINT "42" 1
specs PRINT "3.14" 1
specs PRINT "2+3*4" 1    # gives 14 (standard precedence)
```

### String Literals

Enclosed in single quotes *within* the expression:

```
specs PRINT "'hello' || ' ' || 'world'" 1
```
Output: `hello world`

Note: The outer quotes are the shell's; the inner single quotes are the ALU's string delimiters.

### Field Identifiers

When a field identifier (say `a`) is set, you can use it by name, no colon needed:

```
echo "42" | specs a: w1 . PRINT "a * 2" 1
```
Output: `84`

Test whether a field identifier is set with `present(a)`.

### Counters

Counters are numbered variables, written `#0`, `#1`, `#2`, ... They are initialized to zero and persist across records.

```
echo -e "10\n20\n30" | specs a: w1 1 SET "#0+=a" PRINT "#0" nw
```
Output:
```
10 10
20 30
30 60
```

The counter `#0` accumulates the sum as each record is processed.

See also **[Chapter 9](#chap9)** for the **Named persistent variables** available thgouth the  `pset()` and `pget()` functions.

### Configured Literals

`@name` refers to a value from the configuration file (`~/.specs` or `%HOME%\specs.cfg`):

```
pi: 3.14159265
```

And then the specification:
```
specs PRINT "@pi * 2" 1
```
Output: `6.28318530`

Note: `@pi` used *outside* of an expression (like `specs @pi 1`) is expanded to the raw string `3.14159265` before specs parses it, so it becomes a string literal, not a numeric expression. Inside an expression (inside quotes after `PRINT`, `SET`, `IF`, etc.), it is a numeric value.

### The Entire Record — `@@` and `@!`

- `@@` — the entire current input record (always the original, unaffected by `CONTEXT`)
- `@!` — the current in-context input record (same as `@@` when no `CONTEXT` is active)

```
echo "hello world" | specs PRINT "length(@@)" 1
```
Output: `11`

**Note:** `CONTEXT` is described in [Chapter 13](#chap13)

### Record Offsets — `@+n` and `@-n`

In expressions, `@+n` refers to the record n positions ahead and `@-n` to n positions behind.

## Operators

### Arithmetic Operators

| Operator | Name | Example |
|----------|------|---------|
| `+` | Addition | `3 + 4` → `7` |
| `-` | Subtraction | `10 - 3` → `7` |
| `*` | Multiplication | `3 * 4` → `12` |
| `/` | Division | `17 / 4` → `4.25` |
| `//` | Integer division | `17 // 4` → `4` |
| `%` | Remainder | `17 % 4` → `1` |

Note: specs uses `//` for integer division and `%` for remainder, matching C/Python conventions (opposite to REXX, where `//` is remainder and `%` is integer division).

### String Operator

| Operator | Name | Example |
|----------|------|---------|
| `||` | Concatenation | `'hello' || ' ' || 'world'` → `hello world` |

### Comparison Operators

| Operator | Name | Notes |
|----------|------|-------|
| `<` | Less than | Smart: float if either side is numeric, string otherwise |
| `<=` | Less than or equal | Smart: float if either side is numeric, string otherwise |
| `>` | Greater than | Smart: float if either side is numeric, string otherwise |
| `>=` | Greater than or equal | Smart: float if either side is numeric, string otherwise |
| `=` | Equal | Smart: float if either side is numeric, string otherwise |
| `==` | Strict equal | String comparison only (always) |
| `!=` | Not equal | Smart: float if either side is numeric, string otherwise |
| `!==` | Strict not equal | String comparison only (always) |
| `<<` | String less than | Alphabetical comparison (always) |
| `<<=` | String less than or equal | Alphabetical comparison (always) |
| `>>` | String greater than | Alphabetical comparison (always) |
| `>>=` | String greater than or equal | Alphabetical comparison (always) |

All of `<`, `<=`, `>`, `>=`, `=`, and `!=` use the same **smart** rule: if *either* operand looks like a number, both are compared as floating-point; otherwise they are compared as strings. This means `3 < 3.5` is true (numeric), but `apple < banana` is also true (string alphabetical order).

The **strict** operators (`==`, `!==`, `<<`, `<<=`, `>>`, `>>=`) always compare as strings regardless of whether the operands look like numbers. So `3 == 3.0` is false (different string representations), but `3 = 3.0` is true (same numeric value).

### Logical Operators

| Operator | Name | Effect |
|----------|------|--------|
| `!` | Logical NOT | `!0` → `1`, `!5` → `0` |
| `&` | Logical AND | `1` if both operands are non-zero |
| `|` | Logical OR | `1` if either operand is non-zero |

## Functions

Functions are a powerful part of expressions. They take zero or more arguments and return a result. specs provides a large library of **built-in functions** for string manipulation, mathematics, statistics, and more. You can also write your own functions in **Python** to extend specs's capabilities.

### Built-in Functions

Built-in functions cover:

- **String functions**: `length()`, `substr()`, `pos()`, `substitute()`, `reverse()`, and many more
- **Mathematical functions**: `abs()`, `sqrt()`, `sin()`, `cos()`, `floor()`, `round()`, etc.
- **Statistical functions**: `sum()`, `average()`, `min()`, `max()`, `variance()`, `stddev()`
- **Record access functions**: `word()`, `field()`, `range()`, `record()`, etc.
- **Time functions**: `tf2mcs()`, `mcs2tf()`, `tf2s()`, `s2tf()`
- **Special functions**: `first()`, `eof()`, `getenv()`, `exec()`, and more

**Example — string length:**
```
echo "hello world" | specs PRINT "length(@@)" 1
```
Output: `11`

**Example — trigonometry:**
```
echo 3.14 | specs PRINT "sin(@@)" 1
```
Output: `0.00159265291648682823`

For a complete guide to the built-in functions, see **[Chapter 9: Built-in Functions](#chap9)**.

### User-Written Functions

You can extend specs with your own functions written in **Python**. These custom functions integrate seamlessly into expressions and can perform complex logic that would be difficult with built-in functions alone.

**Example — custom function to count vowels:**
```python
def count_vowels(s):
    return sum(1 for c in s.lower() if c in 'aeiou')
```

For details on writing and using custom Python functions, see **[Chapter 14: Extending specs with Python](#chap14)**.
\newpage

## SET — Storing Values in Counters

The `SET` spec unit stores a value in a counter:

```
SET "#n op expression"
```

Assignment operators:

| Operator | Effect |
|----------|--------|
| `:=` | Assign |
| `+=` | Add |
| `-=` | Subtract |
| `*=` | Multiply |
| `/=` | Divide |
| `//=` | Integer divide |
| `%=` | Remainder |
| `||=` | Append (string concatenation) |

```
echo -e "1\n2\n3" | specs SET "#0:=0" EOF PRINT "#0" 1
```
This is wrong — the `SET` runs for every record, resetting #0 each time. To initialize a counter, use `IF "first()"`:

```
echo -e "2\n3\n4" | specs w1 a: IF "first()" THEN SET "#0:=1" ENDIF SET "#0*=a" EOF PRINT "#0" 1
```
Output: `24` (2 × 3 × 4)

Here, `#0` is initialized to 1 on the first record, then multiplied by each word value. Without the `IF "first()"` guard, the counter would be reset to 1 on every record.

**NOTE:** The control structures such as `IF..THEN..ENDIF` are described in [Chapter 10](#chap10); Spec Units that follow `EOF` are executed once at the end of the run, as explained in [Chapter 11](#chap11). `first()` evaluates to 1 on the first record, and zero (or false) ever after.

### Compound SET

Multiple assignments in one SET:

```
specs SET "#0+=1 ; #1+=a"
```

### Assignments as Expressions

An assignment can appear anywhere an expression is expected. The assignment is performed and the expression evaluates to the result:

```
specs PRINT "#0:=2" 1 /plus/ nextword PRINT "#1:=3" nextword /equals/ nextword PRINT "#1+=#0" nextword
```
Output: `2 plus 3 equals 5`

## Type Coercion

The ALU is not strongly typed. Numbers and strings are interchangeable:

```
specs PRINT "'2+3=' || (2+3)" 1
```
Output: `2+3=5`

When a string is used in a numeric context, specs tries to parse it as a number. Non-numeric strings evaluate to zero in integer context. Floating-point values used as integers are truncated.

## Exactness

Besides its value, every result of an expression carries an **exactness** flag: it is either **exact** (no rounding has occurred) or **inexact**. Integers and strings are normally exact; results of floating-point computations are normally inexact, since they may have lost precision along the way.

```
echo "" | specs PRINT "exact(3+4)" 1 /and/ nextword PRINT "exact(3/4)" nextword
```
Output: `1 and 0`

Here `3+4` is an integer computation, so it is exact, while `3/4` involves floating-point division, so it is inexact.

The built-in `exact()` function, introduced in **[Chapter 9](#chap9)**, lets you test the exactness of an expression. User-written Python functions can also inspect and control exactness explicitly; see **[Chapter 14](#chap14)** for details.

---

# Chapter 9: Built-in Functions {#chap9}

specs provides a library of built-in functions for use in expressions. This chapter presents them organized by category.

## Calling Functions

Functions appear inside ALU expressions:

```
specs PRINT "length('hello')" 1
```
Output: `5`

Multiple arguments are separated by commas:
```
specs PRINT "substr('hello world', 7, 5)" 1
```
Output: `world`

## Passing Arguments

When calling a function, you can pass different kinds of arguments. Here are the options:

### Numeric Literals

Numbers can be passed without quotes:

```
specs PRINT "sqrt(16)" 1
```
Output: `4`

```
specs PRINT "pow(2, 8)" 1
```
Output: `256`

### String Literals with Quotes

Strings containing spaces or special characters must be enclosed in delimiters. You can use single quotes, double quotes, or slashes:

```
specs PRINT "length('hello world')" 1
```
Output: `11`

```
specs PRINT 'substr("hello world", 7, 5)' 1
```
Output: `world`

```
specs PRINT "pos(world, /hello world/)" 1
```
Output: `7`

### Unquoted String Literals

A single word with no special characters can be passed without quotes, as long as it is unambiguous. For example:

```
specs PRINT 'length(hello)' 1
```
Output: `5`

```
$ specs PRINT 'length(hello!)' 1
Error while parsing command-line arguments:
Expression in Token PRINT at index 1 with content <length(hello!)>:
Operator '!' is invalid.
```

However, **you cannot pass a single-character unquoted argument** because **specs** will interpret it as a field identifier reference (see below). These fail:

```
specs PRINT "length(x)" 1              # ERROR: field identifier x not defined
specs PRINT "includes(hello,x)" 1      # ERROR: field identifier x not defined
```

Always quote single-character arguments:

```
specs PRINT "length('x')" 1
```
Output: `1`

### Field Identifiers

When you have defined a field identifier (e.g., `a:`), you can use it in expressions by name, without the colon:

```
echo "hello world" | specs a: WORD 1 . PRINT "length(a)" 1
```
Output: `5`

```
echo "5 3" | specs WORD 1 a: WORD 2 b" PRINT "a * b" 1
```
Output: `15`

### Configured Literals

Configured literals from your `~/.specs` file (or set with `--set`) are referenced with the `@` prefix:

```
specs PRINT "@pi" 1
```
Output: `3.14159265` (if `pi: 3.14159265` is in your config)

```
echo 5 | specs W1 r: PRINT "2 * @pi * r" 1
```
Output: `31.4159265`

### Special Values

Several special values are available in expressions:

- `@@` — the entire current input record
- `@!` — the current in-context input record
- `@+n` / `@-n` — records at offset positions (e.g., `@+1` is the next record)
- `#n` — counters (e.g., `#0` is counter 0)

```
specs PRINT "length(@@)"
```
Output: (length of the current input record)

```
echo "hello world" | specs PRINT "word(1)"
```
Output: `hello`

### Nested Function Calls

Functions can be nested. The inner function is evaluated first:

```
specs PRINT "length(word(1))" 1
```
Output: (length of the first word)

```
specs PRINT "substr(@@, 1, length(word(1)))" 1
```
Output: (the first word)

### Argument Passing Rules Summary

| Argument Type | Example | Notes |
|---------------|---------|-------|
| Number | `sqrt(16)` | No quotes needed |
| Multi-word string | `length('hello world')` | Quotes required |
| Single-word string | `length(hello)` | Quotes optional if unambiguous |
| Single-character string | `length('x')` | Quotes required (else interpreted as field identifier) |
| Field identifier | `length(a)` | Use the letter only, no colon |
| Configured literal | `2 * @pi` | Prefix with `@` |
| Current record | `length(@@)` | Use `@@` for the full record |
| Nested function | `length(word(1))` | Inner function evaluated first |

## Numerical and Logical Functions

| Function | Description |
|----------|-------------|
| `abs(x)` | Absolute value of *x* |
| `ceil(x)` | Smallest integer ≥ *x* |
| `floor(x)` | Largest integer ≤ *x* |
| `round(x, d)` | Round *x* to *d* decimal places (default: nearest integer) |
| `sign(x)` | 1 if *x* > 0, 0 if *x* = 0, -1 if *x* < 0 |
| `sqrt(x)` | Square root of *x* (returns NaN for negative *x*) |
| `exp(x)` | e raised to the power *x* |
| `log(x, base)` | Logarithm of *x* (default: natural log); optional *base* |
| `pow(x, y)` | *x* raised to the power *y* |
| `sin(x)` | Sine of *x* (in radians) |
| `cos(x)` | Cosine of *x* (in radians) |
| `tan(x)` | Tangent of *x* (in radians) |
| `arcsin(x)` | Inverse sine of *x* (result in radians) |
| `arccos(x)` | Inverse cosine of *x* (result in radians) |
| `arctan(x)` | Inverse tangent of *x* (result in radians) |
| `dsin(x)` | Sine of *x* (in degrees) |
| `dcos(x)` | Cosine of *x* (in degrees) |
| `dtan(x)` | Tangent of *x* (in degrees) |
| `arcdsin(x)` | Inverse sine of *x* (result in degrees) |
| `arcdcos(x)` | Inverse cosine of *x* (result in degrees) |
| `arcdtan(x)` | Inverse tangent of *x* (result in degrees) |
| `fact(n)` | *n* factorial. Returns Gamma(*n*+1) if *n* is not a small integer |
| `combinations(n, k)` | Number of ways to choose *k* items from a set of *n* |
| `permutations(n, k)` | Number of ordered ways to choose *k* items from a set of *n* |
| `rand(x)` | Random integer 0 ≤ result < *x*. If *x* omitted: random float [0, 1) |
| `not(expr)` | 1 if *expr* is zero; 0 otherwise |
| `c2d(x)` | Signed decimal value of binary string *x* (up to 8 bytes, 2's complement) |
| `c2u(x)` | Unsigned decimal value of binary string *x* (up to 8 bytes) |
| `c2f(x)` | Floating-point value of binary string *x* |
| `frombin(x)` | Decimal value of binary string *x* (little-endian) |
| `tobin(x)` | Binary (byte) representation of integer *x* |
| `tobine(x, n)` | Binary representation of integer *x* as *n*-byte string |
| `fmt(value, format, digits, decimal, separator)` | Format a number (see below) |
| `pretty(value, flimit, ilimit, locale)` | Format with thousands separators (see below) |

### fmt and pretty

**`fmt(value, format, digits, decimal, separator)`** formats a number with control over the output format:

- *value*: the number to format
- *format*: `f` for fixed-point, `s` for scientific notation
- *digits*: total number of digits in the output
- *decimal*: decimal places (for fixed-point format) or significant digits (for scientific notation)
- *separator*: character to use as thousands separator

**`pretty(value, flimit, ilimit, locale)`** formats a number with thousands separators:

- *value*: the number to format
- *flimit*: threshold for applying formatting to floating-point numbers
- *ilimit*: threshold for applying formatting to integers
- *locale*: locale string to determine separator characters (e.g., `en_US`, `de_DE`)

## String Functions

| Function | Description |
|----------|-------------|
| `length(s)` or `length(s)` | Length of string *s* |
| `left(s, n)` | Leftmost *n* characters of string *s* (space-padded if shorter) |
| `right(s, n)` | Rightmost *n* characters of string *s* (space-padded if shorter) |
| `center(s, n)` / `centre(s, n)` | Middle *n* characters of string *s* (space-padded if shorter) |
| `substr(s, start, len)` | Substring of *s* starting at position *start* for *len* characters |
| `pos(needle, haystack)` | 1-based position of the first occurrence of *needle* in *haystack*; *haystack* defaults to current record |
| `lastpos(needle, haystack)` | 1-based position of the last occurrence of *needle*; *haystack* defaults to current record |
| `includes(haystack, needle1, ...)` | 1 if any *needle* is a substring of *haystack*; *haystack* defaults to current record |
| `includesall(haystack, needle1, ...)` | 1 if all *needles* are substrings of *haystack* |
| `substitute(haystack, needle, subst, max)` | Replace occurrences (see below) |
| `reverse(s)` | Reverse the characters in *s* |
| `strip(s, option, pad)` | Strip leading/trailing characters (see below) |
| `space(s, len, pad)` | Replace internal whitespace (see below) |
| `copies(s, n)` | *s* repeated *n* times |
| `translate(s, tableout, tablein, pad)` | Translate characters (see below) |
| `abbrev(h, n, l)` | 1 if first *l* chars of *n* equal first chars of *h* |
| `compare(s1, s2, pad)` | Index of first mismatch between *s1* and *s2*, or 0 if equal; *pad*: padding character |
| `verify(s, ref, option, start)` | Find first character of *s* not in *ref* (N) or in *ref* (M); *start*: starting position |
| `insert(s, target, pos, len, pad)` | Insert string (see below) |
| `overlay(s1, s2, start, len, pad)` | Overlay string (see below) |
| `delstr(s, start, len)` | Delete *len* characters from *s* starting at *start* |
| `justify(s, len, pad)` | Evenly justify words within *s* to length *len* |
| `lvalue(s, sep)` | Left-hand side of `key=value` string *s* (*sep* defaults to `=`) |
| `rvalue(s, sep)` | Right-hand side of `key=value` string *s* (*sep* defaults to `=`) |
| `xrange(start, end)` | String of all characters from *start* to *end* |
| `x2d(s, len)` | Hex string *s* to decimal (signed if *len* missing or ≤ 0) |
| `rmatch(s, regex, flags)` | 1 if *regex* matches all of *s* |
| `rsearch(s, regex, flags)` | 1 if *regex* matches any substring of *s* |
| `rreplace(s, regex, fmt, flags)` | Replace all *regex* matches in *s* with *fmt* |

### matchFlags for Regular Expression Functions

The optional `flags` argument is a comma-separated list:

| Flag | Effect |
|------|--------|
| `icase` | Case-insensitive matching |
| `not_bol` | `^` does not match start of string |
| `not_eol` | `$` does not match end of string |
| `continuous` | Must match from the first character |
| `first_only` | Replace only the first match (for `rreplace`) |
| `sed` | sed formatting for replacement (for `rreplace`) |
| `no_copy` | Do not copy non-matching sections (for `rreplace`) |

### String Manipulation Functions with Multiple Parameters

**`substitute(haystack, needle, subst, max)`** replaces occurrences of a substring:

- *haystack*: the string to search in
- *needle*: the substring to find and replace
- *subst*: the replacement string
- *max*: maximum number of replacements to make; use `U` to replace all occurrences

**`strip(s, option, pad)`** removes leading and/or trailing characters:

- *s*: the string to strip
- *option*: `B` (both leading and trailing, default), `L` (leading only), `T` (trailing only)
- *pad*: the character to strip (default: space)

**`space(s, len, pad)`** normalizes internal whitespace:

- *s*: the string to process
- *len*: number of occurrences of *pad* to use between words (default: 1)
- *pad*: the character to use as separator (default: space)

**`translate(s, tableout, tablein, pad)`** translates characters according to a mapping:

- *s*: the string to translate
- *tableout*: output characters (the translation table)
- *tablein*: input characters to match (the source table)
- *pad*: padding character for unmatched positions (default: space)

Characters in *tablein* are replaced with corresponding characters from *tableout*. If *tableout* is shorter than *tablein*, unmatched characters are replaced with *pad*.

**`insert(s, target, pos, len, pad)`** inserts a string into another string:

- *s*: the string to insert
- *target*: the string to insert into
- *pos*: position where insertion begins (1-based)
- *len*: length of the insertion (if *s* is shorter, it is padded with *pad*)
- *pad*: padding character (default: space)

**`overlay(s1, s2, start, len, pad)`** overlays one string onto another:

- *s1*: the string to overlay
- *s2*: the base string
- *start*: starting position in *s2* (1-based)
- *len*: length of the overlay (if *s1* is shorter, it is padded with *pad*)
- *pad*: padding character (default: space)

## REXX-Derived String Functions

REXX is a scripting language that was central to IBM's CMS (Conversational Monitor System) and remains an important part of the CMS experience. CMS Pipelines, the inspiration for specs, included many REXX-derived functions for string manipulation and word/field processing. These functions are provided in specs to maintain compatibility with CMS Pipelines scripts and to leverage the proven utility of REXX's capabilities.

| Function | Description |
|----------|-------------|
| `bitand(x, y)` | Bitwise AND of strings *x* and *y* |
| `bitor(x, y)` | Bitwise OR of strings *x* and *y* |
| `bitxor(x, y)` | Bitwise XOR of strings *x* and *y* |
| `find(string, phrase)` | Word number of first occurrence of *phrase* in *string* |
| `index(haystack, needle, start)` | Character position of *needle* in *haystack* (optionally starting at *start*) |
| `wordpos(phrase, string, start)` | Word number where *phrase* begins in *string* (optionally starting at *start*) |
| `words(string)` | Number of blank-delimited words in *string* |
| `subword(string, start, len)` | Substring starting at word *start* for *len* words |
| `wordindex(string, n)` | Character position of word *n* in *string* |
| `wordlength(string, n)` | Length of word *n* in *string* |
| `delword(string, start, len)` | Delete *len* words starting at word *start* in *string* |
| `sword(str, n, sep)` | *n*-th word of *str* using first char of *sep* as separator |
| `sfield(str, n, sep)` | *n*-th field of *str* using first char of *sep* as separator |
| `wordwith(substr)` | First word of current record containing *substr* |
| `wordwithidx(substr)` | Index of first word containing *substr* |
| `fieldwith(substr)` | First field containing *substr* |
| `fieldwithidx(substr)` | Index of first field containing *substr* |

\newpage
## Record Access Functions

These functions read from the current (or context-affected) input record:

| Function | Description |
|----------|-------------|
| `record()` | Entire current input record (context-affected) |
| `cfrecord()` | Entire current input record (ignoring context) |
| `range(n, m)` | Characters *n* through *m* of the current record |
| `word(n)` | *n*-th word of the current record |
| `wordrange(n, m)` | Words *n* through *m* |
| `wordcount(s, p)` | Number of words in *s* (default: current record); *p*: separator pattern |
| `wordstart(n)` | Start position of word *n* |
| `wordend(n)` | End position of word *n* |
| `wordlength(n)` | Length of word *n* |
| `field(n)` | *n*-th field of the current record |
| `fieldrange(n, m)` | Fields *n* through *m* |
| `fieldcount(s, p)` | Number of fields in *s* (default: current record); *p*: separator pattern |
| `fieldindex(n)` | Start position of field *n* |
| `fieldend(n)` | End position of field *n* |
| `fieldlength(n)` | Length of field *n* |
| `recno()` | Current record number (how many read so far) |
| `number()` | Number of *processing cycle* - `1` when `first()` |
| `ctxrecno()` | Record number of the context-affected record |
| `ctxoffset()` | Current context offset (0 if no CONTEXT is active) |
| `ctxoob(s)` | 1 if *s* came from out-of-bounds input |
| `split(sep, hdr, ftr)` | Output fields on separate lines (see below) |
| `splitw(sep, hdr, ftr)` | Output words on separate lines (see below) |
| `splus(s, o, l)` | Substring relative to string (see below) |
| `wplus(s, o, l)` | Substring relative to word (see below) |
| `fplus(s, o, l)` | Substring relative to field (see below) |
| `next()` | Current print position (where next output would go) |
| `rest()` | Columns remaining to terminal width |
**Note:** Despite the similarity in names, `number()` and `recno()` are not synonyms the way that the `NUMBER` and `RECNO` input sources are. `number()` counts cycles, while `recno()` counts input records. As shown in [Chapter 12](#chap12), a cycle can read more than one record.
\newpage

### Record Output and Substring Functions

**`split(sep, hdr, ftr)`** outputs all fields on separate lines:

- *sep*: field separator (default: space)
- *hdr*: header to print before each field (optional)
- *ftr*: footer to print after each field (optional)

**`splitw(sep, hdr, ftr)`** outputs all words on separate lines:

- *sep*: word separator (default: space)
- *hdr*: header to print before each word (optional)
- *ftr*: footer to print after each word (optional)

**`splus(s, o, l)`** extracts a substring at a specified offset from the first occurrence of a string:

- *s*: the string to search for
- *o*: offset from the start of *s* (can be negative to start before *s*)
- *l*: length of the substring to extract

**`wplus(s, o, l)`** extracts a substring at a specified word offset from the first occurrence of a word:

- *s*: the word to search for
- *o*: word offset from *s* (can be negative)
- *l*: number of words to extract

**`fplus(s, o, l)`** extracts a substring at a specified field offset from the first occurrence of a field:

- *s*: the field to search for
- *o*: field offset from *s* (can be negative)
- *l*: number of fields to extract

## Time Functions

| Function | Description |
|----------|-------------|
| `tf2mcs(s, f)` | Parse time string *s* with format *f* → microseconds since epoch |
| `mcs2tf(x, f)` | Format microseconds-since-epoch *x* with format *f* → string |
| `tf2s(s, f)` | Parse time string *s* with format *f* → seconds since epoch |
| `s2tf(x, f)` | Format seconds-since-epoch *x* with format *f* → string |

Time format strings follow strftime conventions. The additional format code `%xf` (where x is 0–6) produces fractional seconds with x digits.

Example:
```
echo "2024-01-15 14:32:07" | specs PRINT "tf2mcs(word(1)||' '||word(2),'%Y-%m-%d %H:%M:%S')" 1
```
Output: `1705329127000000`

\newpage
## Statistical and Frequency Map Functions {#statistical-and-frequency-map-functions}

These pseudo-functions work with **field identifiers** to accumulate statistics across records. They are called pseudo-functions because the only valid argument is a field identifier. The last three are not pseudo-functions. Instead, they work with a special needle-counting map:

| Function | Description |
|----------|-------------|
| `present(a)` | 1 if field identifier *a* is assigned, 0 otherwise |
| `sum(a)` | Sum of all values assigned to field identifier *a* |
| `min(a)` | Minimum value assigned to *a* |
| `max(a)` | Maximum value assigned to *a* |
| `average(a)` | Arithmetic mean of values assigned to *a* |
| `variance(a)` | Variance of values assigned to *a* |
| `stddev(a)` | Standard deviation of values assigned to *a* |
| `stderrmean(a)` | Standard error of the mean of values assigned to *a* |
| `fmap_nelem(a)` | Number of distinct values of *a* |
| `fmap_nsamples(a)` | Total number of samples of *a* |
| `fmap_common(a)` | Most frequent value of *a* |
| `fmap_rare(a)` | Least frequent (but non-zero) value of *a* |
| `fmap_count(a, s)` | Number of times value *s* appeared for *a* |
| `fmap_frac(a, s)` | Fraction of samples of *a* that equal *s* |
| `fmap_pct(a, s)` | Percentage of samples of *a* that equal *s* |
| `fmap_sample(a, s)` | Record *s* as a new sample for *a*, without setting *a*; returns count |
| `fmap_dump(a, format, sortOrder, showPct)` | Formatted dump of frequency map for *a*; see parameters below |
| `countocc(needle, haystack)` | Times *needle* appeared in *haystack* across all calls |
| `countocc_get(needle)` | Count of *needle* without a new match attempt |
| `countocc_dump(format, sortOrder, showPct)` | Dump of countocc frequency map; see parameters below |

### fmap_dump Parameters

The below arguments apply to the `fmap_cump` and the `countocc_dump` functions:

- **format**: `txt`/`0`/empty (default), `lin` (boxed), an integer (fixed width), `csv`, `json`
- **sortOrder**: `s`/`sa` (alphabetical asc, default), `sd` (alphabetical desc), `c`/`ca` (count asc), `cd` (count desc)
- **showPct**: boolean — if true, adds a percentage column
\newpage

Example — distribution of the count of distinct prime factors in natural numbers up to 210:
```
$ seq 210 | specs print "distinct_prime_factors(word(1))" a: EOF PRINT "fmap_dump(a,lin,sa,1)"
+---+-----+--------+
| 0 |   1 |  0.48% |
| 1 |  60 | 28.57% |
| 2 | 116 | 55.24% |
| 3 |  32 | 15.24% |
| 4 |   1 |  0.48% |
+---+-----+--------+
```
Notes:

- 210 is the smallest number with 4 distinct prime factors: `210==2*3*5*7`
- The `distinct_prime_factors` is an [example](#dpf) in [Chapter 14](#chap14).

Example — the sum of the numbers from 1 to 10
```
seq 10 | specs w1 a: EOF PRINT "sum(a)"
```
Output: `55`

## Shell Command Functions

These functions run a shell command and let you process its standard output, standard error, and return code:

| Function | Description |
|----------|-------------|
| `exec(cmd)` | Run shell command *cmd*; returns its standard output (one trailing newline stripped) |
| `exc1(cmd, [lineNo])` | Like `exec`, but returns only the content of output line *lineNo*; *lineNo* must be a positive integer and defaults to 1; empty string if absent |
| `excrc()` | Return code of the last exec/exc1 run; NaN if none has run |
| `excerr()` | Standard error of the last exec/exc1 run; empty string if none has run |

`exec` returns whatever the command wrote to standard output. The command's return code and standard error are saved for the `excrc()` and `excerr()` functions:

```
specs PRINT "exec('echo hello')"
hello
```

`exc1` is the same as `exec`, except it returns just one line of output:

```
specs PRINT "exc1('echo hello')"
hello
```

### Reading the return code and standard error

`excrc()` and `excerr()` always reflect **only the last** shell command that was run. Read them right after the relevant `exec`/`exc1` call, and before any other shell command runs:

```
specs PRINT "exec('grep specs *')" WRITE "stderr" WRITE PRINT "excerr()"
```

Be especially careful when `exec` or `exc1` appear inside an `IF` or `WHILE` block, where the order and number of runs may not be obvious.

### Efficiency

`exec` and `exc1` launch a new shell for every record they are evaluated on. The following runs `ls | wc` once **per input record**, which is wasteful:

```
specs -C ls "File" 1 w8 NW "is one of the" NW PRINT "exc1('ls | wc')" NW "files in this directory"
```

A more efficient version runs the command only once — on the first record — and stores the result in a field identifier for reuse:

```
specs -C ls IF "first()" THEN SET "#0:=exc1('ls | wc')" ENDIF "File" 1 w8 NW "is one of the" NW PRINT "#0" NW "files in this directory"
```

\newpage
## Special Functions

| Function | Description |
|----------|-------------|
| `first()` | 1 during the first iteration, 0 otherwise |
| `eof()` | 1 during the run-out (post-input) cycle, 0 otherwise |
| `conf(key, default)` | Value of configured literal *key*; *default* if missing; NaN if no *default* given |
| `defined(key)` | 1 if configured literal *key* is defined, 0 otherwise |
| `getenv(name)` | Value of environment variable *name*; NaN if undefined |
| `pset(var, value)` | Set persistent variable *var* to *value*; returns *value* |
| `pget(var, default)` | Get persistent variable *var*; returns *default* if not set |
| `pdefined(var)` | 1 if persistent variable *var* is defined |
| `pclear(var)` | Clear persistent variable *var*; returns its former value |
| `string(x)` | Force *x* to be stored as a string |
| `exact(expr)` | 1 if *expr* evaluated without rounding, 0 otherwise |
| `break(a)` | 1 if break level *a* is established this cycle (see Chapter 11) |

### Persistent Variables

`pset` and `pget` store values that persist across separate invocations of specs. They are stored on disk (in the user's home directory or a similar location) and survive between sessions.

```
# Count how many times you've run a script
specs PRINT "pset(runs, pget(runs,0)+1)" 1
```

An abbreviated form: `#varname` is equivalent to `pget('varname')`:

```
# Count how many times you've run a script
specs PRINT "pset(runs, #runs+1)" 1
```

---

# Chapter 10: Control Flow  {#chap10}

Normally, **specs** executes all *spec units* in the *specification* sequentially for every input record. Control flow lets you make decisions and repeat operations.

## Conditions

A **condition** is an expression that evaluates to true or false:

- True: any non-zero, non-NaN, non-empty value
- False: zero, NaN, or empty string

Any ALU expression can be a condition.

## IF / THEN / ELSEIF / ELSE / ENDIF

```
IF "condition" THEN
    spec-block
ELSEIF "condition" THEN
    spec-block
ELSE
    spec-block
ENDIF
```

Rules:

- `THEN` is required after every condition.
- `ELSEIF` blocks are optional; you can have as many as you like.
- The `ELSE` block is optional.
- `ENDIF` is required, except at the end of the specification, or just prior to the *run-out block*. Otherwise, nested conditionals may become ambiguous.

**Simple example**: Print whether each number is odd or even:

```
seq 1 5 | specs a: WORD 1 1 is NEXTWORD IF "a%2" THEN odd NEXTWORD ELSE even NEXTWORD ENDIF
```
Output:
```
1 is odd
2 is even
3 is odd
4 is even
5 is odd
```
\newpage
**Note:** For ease of reading, most examples in this chapter will be in the form of a *spec file*. Always assume that the spec file is named `example`:

```
a: WORD 1         1
   is             NEXTWORD
   IF "a%2" THEN
      odd         NEXTWORD
   ELSE
      even        NEXTWORD
   ENDIF
   
$ seq 1 5 | specs -f example
1 is odd
2 is even
3 is odd
4 is even
5 is odd
```

## WHILE / DO / DONE

```
WHILE "condition" DO
    spec-block
DONE
```

As long as the condition is true, the unit sequence is repeated. 

**Warning**: **specs** has a **while-guard** feature that aborts execution after 5000 iterations to prevent accidental infinite loops. You can disable it with the `--no-while-guard` switch, or set the maximum number of iterations to something else using the `while-guard-limit` *configured literal* if you need a longer loop.

**Example**: Print n asterisks for each input number:

```
a: WORD 1  1 
   SET "#1:=a" 
   WHILE "#1>0" DO 
      /*/  NEXT
      SET "#1-=1" 
   DONE
   
$ echo -e "1\n3\n5" | specs -f example 
1*
3***
5*****
```

(In bash, you need to escape the `*` with `\*` to prevent shell glob expansion.)

**Off-by-one warning**: Using an assignment expression like `WHILE "#1-=1"` as the condition is tricky — the decrement happens before the loop body, so you lose one iteration. Prefer the form `SET "#1:=a"  WHILE "#1>0" DO ... SET "#1-=1" DONE`.

## CONTINUE

`CONTINUE` immediately ends the current processing cycle. Any output already generated becomes the output record for this cycle. If no output has been produced, no record is written. Execution continues with the *next* input record.

This is useful for skipping records that match a condition:

```
# Skip comment lines (lines starting with #)
   IF "word(1)=='#'" THEN 
      CONTINUE 
   ENDIF 
   1-* 1
```

## ABEND — ABnormal END

`ABEND` halts the entire specs run with an error message sent to stderr. The current output record is *not* written. No further records are processed.

```
a: 55-57    NEXTWORD
   IF "a > 120" THEN
       ABEND "Age in record exceeds limit"

$ seq 118 122 | specs W1 55 | specs -f example
118
119
120
ABEND: Age in record exceeds limit
```

`ABEND` only makes sense inside a conditional.

## ASSERT

`ASSERT` is like `ABEND` but takes a condition rather than a message. If the condition is false, specs aborts with a message that includes the text of the failed condition:

```
a: 55-57    NEXTWORD
   ASSERT "a <= 120"

$ seq 118 122 | specs W1 55 | specs -f example
118
119
120
ASSERTION failed: a <= 120
```

Use `ASSERT` for sanity checks that should always be true. Use `ABEND` for conditions you want to handle with a specific error message.

**Note:** In both of the above examples, the value **121** triggered the `a > 120` condition or the failure of the `a <= 120` assertion. The reason we never see that value printed is because the abnormal end of **specs** left that last output line unprinted. To demonstrate this, we'll add a `WRITE` keyword that flushes that buffer. See [Chapter 12](#chap12).

```
a: 55-57    NEXTWORD
   WRITE
   ASSERT "a <= 120"

$ seq 118 122 | specs W1 55 | specs -f example
118
119
120
121
ASSERTION failed: a <= 120
```

## SKIP-WHILE and SKIP-UNTIL

These cause specs to skip input records while a condition holds (or until it holds). Once the condition for passing is satisfied once, future records are never re-evaluated — all subsequent records pass through.

```
SKIP-WHILE "condition"    # skip while condition is true
SKIP-UNTIL "condition"    # skip until condition becomes true
```

They work like `CONTINUE` within an implicit `IF`, but they "turn off" after the first record passes.

**Example**: Print all records from the first one whose date is after July 1st, 2020:

```
   WORD 1   a:        # date in yyyymmdd format
   SKIP-WHILE "a < 20200701"
   1-*      1
```

These spec units usually make sense at the beginning of a specification.

---

# Chapter 11: Run-In, Run-Out, and Control Breaks  {#chap11}

## The Normal Cycle

Each iteration of **specs** processes one input record. The *spec units* run, output is produced, and the next record is read. This continues until input is exhausted.

## Run-In: The First Iteration

The first iteration is called the **run-in cycle**. You can specify actions for the *run-in cycle* using the `first()` function. This is useful for setting up counters, printing headers, or recording the first record's values:

```
# Print only records that start with the same word as the first record
IF "first()" THEN
    SET "#0:=word(1)"
ENDIF
IF "#0 == word(1)" THEN
    1-* 1
ENDIF
```

## Run-Out: After the Last Record

The **run-out** cycle runs after the last input record is exhausted. It is used for summaries, totals, and final reports. There are two ways to trigger it.

### The `eof()` Function — Forced Run-Out Cycle

When specs encounters an `eof()` function call anywhere in the specification, it performs a *forced* run-out cycle — an extra iteration after the last record is processed. This cycle has no input record (the record is empty), and `eof()` returns 1.

```
# Print a count of records
IF "eof()" THEN
    /Total records:/ 1 PRINT "#0" nextword
ELSE
    SET "#0+=1"
ENDIF
```

### The `EOF` Keyword

The `EOF` keyword marks a group of spec units that run only during the run-out cycle. The spec units following `EOF` are only executed after all input records have been processed.

```
# Sum and report
a: word 1 1
   set "#0+=a"
EOF
   /Total:/ 1
   print #0 NEXTWORD
```

Given input `1`, `2`, `3`, `4`, output is:
```
1
2
3
4
Total: 10
```

### `first()` vs `eof()` vs. `EOF`

Both `eof()` and `EOF` accomplish similar things: The difference is that when using `eof()`, the other *spec units* in the normal cycle also run.

- Use `EOF` when the run-out logic is clearly separated from the per-record logic — it reads cleanly in a spec file.
- Use `eof()` inside `IF` when the logic is interleaved with per-record processing.

For comparison:
```
$ seq 3 | specs IF "first()" THEN 1st ENDIF "cycle" NW a: W1 IF "eof()" THEN "?sum(a)"
1st cycle 1
cycle 2
cycle 3
cycle 6

$ seq 3 | specs IF "first()" THEN 1st ENDIF "cycle" NW a: W1 EOF "?sum(a)"
1st cycle 1
cycle 2
cycle 3
6
```
So,

- A *run-in cycle* with `first()` is a normal cycle with an input record. If you want to print headers, use `WRITE` to place them on a different line from the one with the output associated with the first input record.
- A *run-out cycle* with `eof()` forces an extra cycle, with no input record. Other *spec units* **will** run.
- A *run-out cycle* with `EOF` runs only a truncated cycle with the *spec units* that follow the `EOF` keyword.

## Control Breaks

A **control break** occurs when a tracked *field identifier* changes value from one record to the next. This is a pattern common in report generation — printing a header whenever a category changes, and suppressing repeated values.

A control break is conceptually similar to the run-out cycle triggered by `EOF`: both let you set aside a group of spec units that only run under a special condition, rather than on every record. Where `EOF` triggers once, when input is exhausted, a control break can trigger on *any* record, whenever a chosen field identifier's value changes.

### The `BREAK` Keyword

Consider this CSV file of personnel records (department, first name, last name), `employees.csv`:
```
+-------------------------+
| Payroll,Eddy,Eck        |
| Payroll,Janel,Polen     |
| Finance,Leonard,Cockett |
| Finance,Dorie,Lugo      |
| Finance,Wiley,Cheung    |
| R&D,Jamaal,Mcgillis     |
| R&D,Jonna,Scheffer      |
| Sales,Kristopher,Lind   |
| Sales,Margret,Picone    |
+-------------------------+
```

Suppose we want to print this out, but suppress repetition of the department field. We track the department in a field identifier, and place a `BREAK` statement in front of the spec units that should run only when that field identifier's value has just changed:

```
    IF "first()" THEN
        Dept          1
        "Last, first" 10
        WRITE
        "======="           1
        "=================" 10
        WRITE
    ENDIF
    FIELDSEPARATOR ,
    c: FIELD 1   .
       FIELD 3  10
       /,/      NEXT
       FIELD 2  NEXTWORD
    BREAK c
        ID c 1
```

Here, `c` is set to the first field (department name) of each CSV record. Spec units after `BREAK c` — in this case, `ID c 1`, which prints the department name — only run on records where `c` has just changed from the previous record. For this to work, the input needs to be sorted. **specs** processes records one at a time, so it cannot sort on its own.

Result:
```
Dept     Last, first
=======  =================
Payroll  Eck, Eddy
         Polen, Janel
Finance  Cockett, Leonard
         Lugo, Dorie
         Cheung, Wiley
R&D      Mcgillis, Jamaal
         Scheffer, Jonna
Sales    Lind, Kristopher
         Picone, Margret
```

## Break Levels

When the value of a field identifier changes from one record to the next, the **break level** is **set** to that identifier. In the example above, when the *specification* encountered a new department name, *break level* `c` was set. That identifier, along with all identifiers alphabetically lower than it, are said to be **established** for the current record.

For example: if field identifiers `a`, `b`, and `c` are all tracked (each with its own assignment earlier in the spec), and `b` changes value while `a` does not, then break levels `a` and `b` are established, but `c` is not. Capital letters are considered higher than lower-case letters when comparing identifiers.

Spec units following a `BREAK x` statement execute only when break level `x` is established for the current record. Since establishment cascades downward, `x` is established either when `x` itself just changed, or when some alphabetically higher tracked identifier just changed (which establishes `x` along with it). `x` is *not* established merely because some lower identifier changed. This lets you nest breaks: a `BREAK a` group runs on every break, however minor, since some identifier changes whenever `a` is established; a `BREAK c` group runs only on the less frequent, more specific occasions when `c` itself changes.

### The `break()` Function

The `break(a)` function returns 1 if break level `a` is established for the current record, and 0 otherwise. Unlike `BREAK`, which applies to every spec unit that follows it, `break()` is a pseudo-function you can use inside an `IF` condition, so it can be combined with other logic. (It is called a pseudo-function because its only allowed argument is a field identifier.)

```
specs
    FIELDSEPARATOR ,
    a: FIELD 1   .
    IF break(a) THEN
        ID a           1
        /Department:/ NW
        WRITE
    ENDIF
    FIELD 3   10
    /,/        N
    FIELD 2   NW
```

This produces a header line for each department, followed by employees:

```
Payroll Department:
         Eck, Eddy
         Polen, Janel
Finance Department:
         Cockett, Leonard
         Lugo, Dorie
         Cheung, Wiley
R&D Department:
         Mcgillis, Jamaal
         Scheffer, Jonna
Sales Department:
         Lind, Kristopher
         Picone, Margret
```

## PRINTONLY and KEEP

Now that break levels are understood, we can use them to control whether a record is printed at all, not just which spec units run.

`PRINTONLY` (a **MainOption**) suppresses per-record output unless a specified break level is established. It is followed by either:

- A **field identifier** (case matters) — records are suppressed until that break level is established (as described above under Break Levels)
- The keyword **`EOF`** — records are suppressed until the input is exhausted (i.e., only the run-out output is printed)

```
PRINTONLY EOF
# ... per-record spec units (all output suppressed) ...
EOF
   # ... summary output (printed) ...
```

`KEEP` (always following `PRINTONLY`) prevents the output buffer from being cleared between non-printed records. This allows output from multiple records to accumulate into a single output record before the break level is established, instead of being discarded record by record.

### PRINTONLY Without KEEP — Picking One Record per Group

Without `KEEP`, a suppressed record's output is simply thrown away before the next record is read. This is useful when you only care about *one* record per group and want to discard the rest — for example, printing only the first employee listed for each department (imagine the data is sorted so that the department head is listed first):

```
PRINTONLY c
FIELDSEPARATOR ,
c: FIELD 1   1
   FIELD 3  10
   /,/      NEXT
   FIELD 2  NEXTWORD
```

Since `c` (the department) is tracked automatically as soon as it's assigned, no explicit `BREAK` is needed: `PRINTONLY c` prints a record only when break level `c` is established — that is, only on the first record of each new department. Every other record in that department is fully suppressed and discarded, buffer and all, because there is no `KEEP`.

Given the same personnel CSV from earlier, sorted by department, the output is:
```
Payroll  Eck, Eddy
Finance  Cockett, Leonard
R&D      Mcgillis, Jamaal
Sales    Lind, Kristopher
```

Only the department's first row survives; the rest never produce output at all.

### PRINTONLY with KEEP — Accumulating Records into One Line

With `KEEP`, the output buffer is *not* cleared when a record is suppressed, so spec units in the next cycle keep writing into the same buffer, picking up right where the previous (suppressed) record left off. This lets you build up a single output line piece by piece across many records, and only flush it once, when a record finally isn't suppressed.

Combined with `PRINTONLY EOF`, this lets you accumulate something from every input record and print it as a single line at the very end:

```
PRINTONLY EOF
KEEP
FIELDSEPARATOR ,
a: FIELD 3    NEXTWORD
EOF
   /(end)/ NEXTWORD
```

Every record's last name (`FIELD 3`) is placed with `NEXTWORD`, right after whatever is already in the buffer. Because of `KEEP`, none of that is discarded between records — it just keeps growing. Only at `EOF`, when the record is no longer suppressed, does the accumulated line finally get written, with `(end)` tacked on:

```
Eck Polen Cockett Lugo Cheung Mcgillis Scheffer Lind Picone (end)
```

Compare this with the same specification *without* `KEEP`:

```
PRINTONLY EOF
FIELDSEPARATOR ,
a: FIELD 3    NEXTWORD
EOF
   /(end)/ NEXTWORD
```

Now every suppressed record's contribution to the buffer is discarded before the next record is read, so by the time `EOF` runs, all that's left is whatever the `EOF` spec units themselves produced:

```
(end)
```

This contrast is the key reason `KEEP` exists: use it whenever the *point* of `PRINTONLY` is to gather output across records rather than to simply pick which one record gets to print.

### A Numerical Example
```
$ seq 10 | specs PRINTONLY EOF a: WORD 1 EOF "?sum(a)"
55

$ seq 10 | specs PRINTONLY EOF KEEP a: WORD 1 EOF "?sum(a)"
1 2 3 4 5 6 7 8 9 10 55
```
Notes:

- The `a: WORD 1` *data field* has an implied `NEXTWORD` *output placement*.
- `a` is counted towards the `sum` function even when no break is established.

### An Advanced Example

**Question:** Can you figure out why the following specification is producing this output?
```
PRINTONLY a KEEP 
a: PRINT "floor(sqrt(word(1)))"

$ seq 81 | specs -f example
1
1 1 2
2 2 2 2 3
3 3 3 3 3 3 4
4 4 4 4 4 4 4 4 5
5 5 5 5 5 5 5 5 5 5 6
6 6 6 6 6 6 6 6 6 6 6 6 7
7 7 7 7 7 7 7 7 7 7 7 7 7 7 8
8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 9
```
\newpage
Compare and contrast with this version, that is a little nicer, but more complex:
```
   IF "first()" THEN
      SET "#0:=''"
   ENDIF
   PRINT "floor(sqrt(word(1)))" a: 
   IF "break(a)" THEN 
      PRINT "#0"      1 
      SET "#0:=a" 
   ELSE 
      SET "#0:=#0||' '||a"

$ seq 81 | specs -f example

1 1 1
2 2 2 2 2
3 3 3 3 3 3 3
4 4 4 4 4 4 4 4 4
5 5 5 5 5 5 5 5 5 5 5
6 6 6 6 6 6 6 6 6 6 6 6 6
7 7 7 7 7 7 7 7 7 7 7 7 7 7 7
8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8
9
```

---

# Chapter 12: Multiple Records and Streams {#chap12}

The default one-in-one-out model can be broken in many ways. This chapter covers that.

## Multiple Output Records — WRITE

Use the `WRITE` spec unit to emit the current output record and start a fresh one, all within the same cycle:

```
specs --inCmd "ls -l"
    /Filename:/ 1
    W-1         NW   # Last word in "ls -l" output is file name
    WRITE            # Skip to next line
    /type:/ 1
    IF "range(1,1)=='d'" THEN
        /directory/ nw
    ELSE             # if we wanted to be fancy, we'd add an ELSEIF for 'l' and /link/
        /file/ nw
    ENDIF
```

`WRITE` flushes the current output record and clears the output buffer. The last output record in a cycle is always written automatically — you don't need a trailing `WRITE`. However, if the last output record is empty, it is not emitted, so there's no harm in a trailing `WRITE`.

## Suppressing Output — NOWRITE and NOPRINT

`NOWRITE` (synonymous with `NOPRINT`) suppresses the automatic output record at the end of the cycle. No output is written for this record unless you used an explicit `WRITE` earlier.

Note that a cycle with no output data fields never emits a record anyway, so `NOWRITE` only matters when the cycle has data fields that emit output. The example below would not demonstrate `NOWRITE` without that "`'LOG: ' 1`" *data field*:

```
# Prefix every line with "LOG: ", but only write lines that contain "error"
specs
    'LOG: ' 1
    IF "includes(,'error')" THEN
        1-*
    ELSE
        NOWRITE
    ENDIF
```

Without the `NOWRITE`, every record would be emitted as "`LOG: <record>`" or "`LOG:`", because the "`'LOG: ' 1`" *data field* emits output even if the `IF` condition doesn't hold. The `NOWRITE` in the `ELSE` branch suppresses that output for records that don't contain "error".

## Multiple Input Records — READ and READSTOP

`READ` and `READSTOP` consume the next input record, making it the new active record for subsequent spec units.

- `READ`: If there are no more records, continue as if an empty record was read.
- `READSTOP`: If there are no more records, stop processing immediately.

Both reset the context offset to zero (the current record) after reading.

**Use case**: Combine multiple input lines into one output line:

```
# Join each pair of lines: "line1 line2"
specs
    a: 1-* 1
    READ
    1-* nw
```

**Use case**: Process git log (variable-length entries):

```
specs -C "git log"
    IF "first()" THEN
        SET "#4:=word(2)"    # save the first commit hash
    ELSE
        PRINT "#4" 1         # print the saved hash
        ASSERT "word(1)=='Author:'"
        WORD 2-*   NEXTWORD  # print the author's name
        READ
        ASSERT "word(1)=='Date:'"
        WORD 2-6 tf2s "%c" NEXTWORD
        WHILE "word(1)!='commit'" DO
            READSTOP
        DONE
        SET "#4:=word(2)"    # save the next commit hash
    ENDIF
```

## UNREAD — Pushing Back a Record

`UNREAD` pushes the current record back so it will be re-read as the first record of the next cycle. This avoids the "consumed one record too many" problem when looping with `READ`/`READSTOP`. It also means we don't need the `#4` variable from the previous example to carry the commit hash across cycles — each cycle can simply start on its own "commit" line:

```
specs -C "git log"
    WORD 2                     1         # commit hash
    READ
    ASSERT "word(1)=='Author:'"
    WORD 2-*            NEXTWORD         # author name and email
    READ
    ASSERT "word(1)=='Date:'"
    WORD 2-6 tf2s "%c" NEXTWORD          # date
    WHILE "word(1)!='commit'" DO
        READSTOP
    DONE
    UNREAD                               # push back the next "commit" line
```
\newpage

## REDO — Two-Phase Processing

`REDO` takes the current output record, makes it the new current input record, and continues executing the specification from the point after `REDO`. This lets you process a record in two passes without piping through a second specs command:

```
# Extract field after the colon, then get just its first word
grep "^Name:" file.txt | specs fs : f2-* 1 REDO w1 1
```

Without `REDO`:
```
grep "^Name:" file.txt | specs fs : f2-* 1 | specs w1 1
```

Both produce the same result, but `REDO` keeps it in one invocation.

## SPLITW and SPLITF — Splitting Records

`SPLITW` splits the current input record into one output record per **word**.
`SPLITF` splits the current input record into one output record per **field**.

```
echo "one two three" | specs splitw 1
```
Output:
```
one
two
three
```

Spec units appearing **before** the split form a prefix replicated in every output record. Spec units appearing **after** the split are applied to each split piece individually.

With a prefix:
```
echo "one two three" | specs /prefix:/ 1 splitw nextword
```
Output:
```
prefix: one
prefix: two
prefix: three
```

Combined with REDO:
```
echo "the boy went to the store" | specs splitw 1 redo /WORD:/ 1 1-* next
```
Output:
```
WORD:the
WORD:boy
WORD:went
WORD:to
WORD:the
WORD:store
```
\newpage

### Optional Separator and OF Clause

Both `SPLITW` and `SPLITF` accept:

- `WS char` or `FS char` — use a specific separator character
- `OF range` — split only the specified portion of the record

```
echo "The numbers are one:two:three and that is all" | specs splitf fs : of 17:29 1
```
Output: `one`, `two`, `three`

The `OF` clause accepts the same syntax as `SUBSTRING`: character ranges, word ranges, or field ranges.

**Restrictions**: Nested `SPLITW`/`SPLITF` in the same spec are not allowed. A `SPLITW` with a `FIELDSEPARATOR` option (or `SPLITF` with `WORDSEPARATOR`) is an error.

## Second Reading Station — SELECT SECOND / SELECT FIRST

At the end of each cycle, specs saves the current input record in a buffer called the **second reading station**. During the next cycle, you can switch to it with `SELECT SECOND`, then return to normal input with `SELECT FIRST`:

```
# Combine each record with the previous one
specs
    WORD 1          1
    SELECT SECOND
    WORD 1 NEXTWORD
    SELECT FIRST
    WORD 2 NEXTWORD
    SELECT SECOND
    WORD 2 NEXTWORD
```

Key notes:

- The second reading station lags one record behind the primary stream.
- Accessing the second reading station forces a run-out cycle (same as using `eof()`).
- `READ` and `READSTOP` must not be used during secondary reading.
- At the start of each new cycle, the primary stream is always selected.
- The rolling context window described in [the next chapter](#chap13) is a more advanced capability than the second reading station.
\newpage

## Multiple Input Streams {#multstrm}

The `--is2` through `--is8` switches assign additional input files. At each cycle, one record is advanced from every stream, even from streams the specification never reads from.

```
specs -i file1.txt --is2 file2.txt WORD 1 1 WORD 2 NW SELECT 2 WORD 2 NW
```

This combines the second column of record *n* from `file1.txt` with the second column of record *n* from `file2.txt`, for each *n*, into a three-column output. The two streams are paired only by their position within the cycle — **specs** does not match records by comparing field values the way a database join would; it is up to you to ensure that matching records at the same offset from different streams makes sense.

`SELECT n` (where n is the stream number 1–8) switches between streams. The stream resets to #1 at the start of each new cycle.

**When all streams have equal record counts**, this positional pairing is straightforward. If stream lengths differ, the `STOP` MainOption controls behavior:

- `STOP ALLEOF` (default): continue until *all* streams are exhausted; **specs** treats READs from exhausted streams as if they return empty records
- `STOP ANYEOF`: stop when *any* stream runs out
- `STOP n`: stop when stream `n` runs out

## Multiple Output Streams

The `--os2` through `--os8` switches assign additional output files. `OUTSTREAM n` (or `OUTSTREAM STDERR`) switches between them. The active output stream resets to #1 at the start of each new cycle:

```
specs -o main.txt --os2 errors.txt
    IF "includes(,'ERROR')" THEN
        OUTSTREAM 2
    ENDIF
    1-* 1
```

Note that the above example switches output stream for the error records, so the error records go into `errors.txt`, but not to `main.txt`.  If we wanted to write all the records to `main.txt`, we could do it like this:
```
specs -o main.txt --os2 errors.txt
    1-* 1
    IF "includes(,'ERROR')" THEN
        WRITE        # Flush the primary stream
        OUTSTREAM 2
        1-* 1        # Write again - this time to the secondary stream
    ENDIF
```

---

# Chapter 13: Rolling Context  {#chap13}

## The Problem

Sometimes you need to look at records other than the current one. For example:

- Detect a change between adjacent records (difference between consecutive values)
- Print a record together with its predecessor and successor
- Process a multi-record "block" as a unit

The `CONTEXT` spec unit provides a general solution.

## The CONTEXT Spec Unit

`CONTEXT n` changes the active input record to the one at offset n from the current record:

- `CONTEXT 1` — look ahead one record (the next record)
- `CONTEXT -1` — look back one record (the previous record)
- `CONTEXT 0` — reset to the current record
- `CONTEXT 3` — look ahead three records

After a `CONTEXT` spec unit, all subsequent input parts (character ranges, word ranges, etc.) read from the offset record instead of the current one. Multiple `CONTEXT` tokens can appear in a single spec.

```
specs 1-* 1 CONTEXT 1 1-* NEXTWORD
```

On input `alpha\nbeta\ngamma`, the output is:
```
alpha beta
beta gamma
gamma
```

Looking backward:
```
specs 1-* 1 CONTEXT -1 1-* NEXTWORD
```
Output:
```
alpha
beta alpha
gamma beta
```

When the context record doesn't exist (before the first record or past the last), **specs** behaves as if it retrieved an empty record.
\newpage

## Multiple CONTEXT Tokens

You can switch context multiple times in one specification:

```
# Print previous, current, and next record's first word
specs CONTEXT -1 WORD 1 1 CONTEXT 0 WORD 1 NEXTWORD CONTEXT 1 WORD 1 NEXTWORD
```

On input `alpha`, `beta`, `gamma`:
```
 alpha beta
alpha beta gamma
beta gamma
```

(Empty context gives an empty field, hence the leading space on the first line.)

## Context in Expressions — @+n and @-n

In expressions, `@+n` refers to the record n positions ahead and `@-n` to n positions behind:

```
specs PRINT "length(@+1)" 1
```

On input `AB`, `CDE`, `F` — outputs `3`, `1`, `0` (the lengths of the next record).

Notes: 

- Reading out-of-bounds with `@+n` or `@-n` does not stop processing, even if `READSTOP` is present.
- The `@+n` / `@-n` elements in expressions are not affected by global context. The following *specification* has exactly the same output as the one above:

```
specs CONTEXT 2 PRINT "length(@+1)" 1
```

## @@ vs. @! and record() vs. cfrecord()

When `CONTEXT` is active, there is an important distinction:

| Expression | Returns |
|------------|---------|
| `@@` | Original current record (ignoring CONTEXT) |
| `@!` | Context-affected record |
| `record()` | Context-affected record (same as `@!`) |
| `cfrecord()` | Original current record (same as `@@`) |

Example:
```
specs CONTEXT 1 PRINT "@!" 1 WRITE PRINT "@@" 1 WRITE
```

On input `alpha\nbeta\ngamma`:
```
beta
alpha
gamma
beta

gamma
```
\newpage

## The ctxrecno() Function

`ctxrecno()` returns the record number that the context record would have if it were the current record:

```
specs PRINT "ctxrecno()" 1 CONTEXT 1 PRINT "ctxrecno()" NEXTWORD
```

On three input records:
```
1 2
2 3
3 4
```

## Out-of-Bounds Records

When `CONTEXT` refers to a position before the first record or past the last, the context record is an **out-of-bounds** (OOB) record. It behaves like an empty string, but carries a hidden OOB flag you can test with `ctxoob()`:

```
specs CONTEXT 2 PRINT "ctxoob()" 1
```

On the last two records of a three-record input, this outputs `0`, `0`, `1` (wait — actually on the third record `CONTEXT 2` would look two ahead which is past the end, so `ctxoob()` returns 1).

**Important**: The OOB flag is preserved when you use the OOB record directly in input parts or expressions. It is **not** preserved after:

- Assignment into a numbered counter (`SET "#n:=record()"`)
- Values that pass through an output-producing spec unit

Query OOB status as close as possible to where the OOB record is read.

## How Rolling Context Works

specs determines the maximum forward and backward context offsets at compile time. It maintains a **sliding window** of records:

- A **forward buffer** of up to the maximum forward offset
- A **backward buffer** of up to the maximum backward offset

With `CONTEXT 3`, specs reads three records ahead before processing begins. This is automatic and transparent.

In verbose mode (`-v`), specs reports the buffer sizes:
```
specs: Using a 3-record rolling context: 2 records forward and 1 records backward.
```

## Restrictions

1. Rolling context is not compatible with threading (`-t` flag).
2. Rolling context is not compatible with multiple input streams.
3. The integer offset of the context record, whether it appears as a `CONTEXT ±n` spec unit or as a `@±n` expression, is limited to an absolute value of 256.
\newpage

## When to Use CONTEXT vs. READ/UNREAD

| Situation | Better approach |
|-----------|----------------|
| Look at the next record without consuming it | `CONTEXT 1` |
| Look at the previous record | `CONTEXT -1` |
| Look several records ahead or behind | `CONTEXT n` |
| Consume multiple records in one cycle | `READ` / `READSTOP` |
| Back up after consuming one too many | `UNREAD` |
| Process variable-length blocks | `READ` + `WHILE` + `UNREAD` |

---

# Chapter 14: Python Functions  {#chap14}

Although **specs** has a rich library of built-in functions, sometimes you need logic that isn't available. You can use the powerful language features of Python, and the [seemingly endless library of modules](https://xkcd.com/353/) to give extra functionality to your *specifications*. This chapter explains to to extend **specs** by writing **external functions** in Python.

Note that not all installations of **specs** include Python support. The following command will show you if Python function support is enabled in your installation:
```
$ specs @python
Enabled`
```
If it isn't, check [Appendix C](#appendixc) to see how you can get a version that supports Python.

Write a Python function when:

- You need an algorithm that's easier to express in Python (complex string processing, external libraries, stateful computations)
- You want to use Python's standard library (`re`, `math`, `datetime`, etc.)
- You need to maintain state across records that's more complex than a counter

## The `localfuncs.py` File

All Python functions must be defined in a file called **`localfuncs.py`**. This file must reside in a directory on the **SPECSPATH** (see Chapter 4 and 5). It is a regular Python file that you can `import` from the Python environment.

All functions whose names **do not** begin with an underscore (`_`) are automatically available to **specs**. Use underscore-prefixed names for helper functions that you don't want to expose.

## A First Example

Suppose you want a `commas` function that formats integers with thousands separators:

```
def commas(x):
    '''Convert the integer x into a string with thousands groups separated by commas'''
    ret = ""
    while x >= 1000:
        rm = str(x % 1000)
        x = x // 1000
        ret = "," + rm + ret
    if x > 0:
        ret = str(x) + ret
    return ret
```

Place this in `$HOME/specs/localfuncs.py` or in `%HOME%\specs\localfuncs.py`. Now use it in **specs**:

```
$ echo "1398234" | specs PRINT "commas(word(1))" 1.12 right
```
Output: `   1,398,234`

## A Bug

Unfortunately, bugs are part of life for those who write code, so:

```
$ echo 2048 | specs PRINT "commas(word(1))"
```
Output: `2,48`

That is not quite what we intended. The problem is that the `rm = str(x % 1000)` line could end up with an `rm` value that is not three-digit. But this function requires three-digit groups. So we need to pad it with zeros. But wait! there's more:

```
$ echo 9003.14159265 | specs PRINT "commas(word(1))"
```
Output: `9.0,3.1415926499994384`

Can you figure out what happened here?

Here's a fixed version:

```
def commas(x):
    '''Convert the integer x into a string with thousands groups separated by commas'''
    try:
        x = int(float(x)) # x will usually be a string. Converting directly to int fails.
    except:
        return 0

    ret = ""
    while x >= 1000:
        rm = str(x % 1000)
        x = x // 1000
        while len(rm) < 3:  # Pad with zeros to length 3
            rm = "0" + rm
        ret = "," + rm + ret
    if x > 0:
        ret = str(x) + ret
    return ret
```

Yes, this works:
```
$ echo -e "2048\n9003.14159265\nhello" | specs PRINT "commas(word(1))"
2,048
9,003
0
```
\newpage

## A more elaborate example {#dpf}

Given an integer n, how many distinct prime factors does it have?  While **specs** contains many built-in functions, it doesn't cover all needs, so if you happen to need the number of distinct prime factors, here's one possible implementations
```
def distinct_prime_factors(n):
    '''Return the number of distinct prime factors of n'''
    n = int(n)                  # Sanitize input
    factor_count = 0            # initialization
    d = 2                       # smallest prime
    include_d = False           # Do we count d?
    while d <= n:
        while n % d == 0:       
            include_d = True
            n //= d             # divide until d is no longer a factor
        if include_d:
            factor_count += 1   # Count each factor only once
        d += 1                  # really only want prime d, but this works
        include_d = False
    return factor_count
```

## Importing Python Modules

Your functions can use any module available in the Python environment:

```
import math
import datetime

def circle_area(radius):
    '''Return the area of a circle with the given radius'''
    return math.pi * float(radius) ** 2

def days_since(date_str):
    '''Return the number of days elapsed since the given date (YYYY-MM-DD)'''
    then = datetime.datetime.strptime(date_str, '%Y-%m-%d')
    return (datetime.datetime.now() - then).days
```
\newpage

## Stateful Functions

**specs** imports the `localfuncs` module only once per run, so module-level variables persist across calls and can be used to maintain state:

```
running_max = None
def update_max(x):
    '''Return the running maximum value seen so far'''
    global running_max
    x = float(x)
    if running_max is None or x > running_max:
        running_max = x
    return running_max
```

Or, for frequency counting (the `countocc` pattern from the docs):

```
occ_dict = dict()
def countocc_py(haystack, needle):
    '''Count occurrences of needle in haystack across all calls'''
    global occ_dict
    if needle not in occ_dict:
        occ_dict[needle] = 0
    if haystack.find(needle) >= 0:
        occ_dict[needle] += 1
    return occ_dict[needle]
```

## Docstrings and Getting Help

Python's docstrings integrate with specs's help system. Document your functions as you would any Python function:

```
def commas(x):
    '''Convert the integer x into a string with thousands groups separated by commas'''
    ...
```

There are two ways to ask specs about your Python functions: `--help pyfuncs` lists **all** of them at once, while `--help <name>` shows the full detail for **one** function by name.

```
specs --help pyfuncs
```
Output:
```
Python Interface Functions:
===========================
- commas (x) : Convert the integer x into a string with thousands groups separated by commas
```

```
specs --help commas
```
Output:
```
commas (x) : Convert the integer x into a string with thousands groups separated by commas
```

For a function with a single-line docstring like `commas`, both forms show the same text. The difference matters once a docstring spans multiple lines, as is common when a function needs a longer explanation:

```
def prime_factors(n):
    '''
    Return the prime factors of n as a comma-separated string.

    Uses trial division up to sqrt(n).
    '''
    ...
```

`specs --help pyfuncs` prints a compact, one-line-per-function summary, so it shows only the **first non-blank line** of each docstring (here, the docstring's first line is blank, so the summary is taken from the next one):

```
$ specs --help pyfuncs

Python Interface Functions:
===========================
- commas (x) : Convert the integer x into a string with thousands groups separated by commas
- prime_factors (n) : Return the prime factors of n as a comma-separated string.
```

`specs --help prime_factors`, on the other hand, prints the **entire** docstring, since you asked about that one function specifically:

```
$ specs --help prime_factors
prime_factors (n) :

    Return the prime factors of n as a comma-separated string.

    Uses trial division up to sqrt(n).
```

## Python Function Options

### --pythonFuncs on/off/auto

Controls when Python functions are loaded:

- `auto` (default): Load Python only if specs encounters an unknown function name. This avoids the overhead of initializing Python for specifications that don't need it.
- `on`: Always load Python, even if no Python functions are called.
- `off`: Never load Python. Calling a Python function is an *unrecognized function* error.

### --pythonErr throw/NaN/zero/nullstr

Controls what happens when a Python function raises an exception:

- `throw` (default): specs aborts with an error message echoing the Python error.
- `NaN`: Return NaN and continue.
- `zero`: Return the integer 0 and continue.
- `nullstr`: Return an empty string and continue.

Use `NaN` or `nullstr` when you want specs to tolerate Python errors gracefully.

## Exactness in Python Functions

specs tracks whether numerical values are **exact** (no rounding has occurred) or **inexact**. This is used by the `exact()` function to let you reason about precision.

### Return Exactness

By default:

- Integer return values are **exact**
- String return values are **exact**
- Float return values are **inexact**

To override, return a 2-tuple `(value, bool)` instead of a plain value:

```
def exact_pi():
    '''Return an exact value of pi'''
    return (3.141592653589793, True)   # exact

def inexact_sqrt():
    '''Return an inexact square root'''
    return (2.23606797749979, False)   # inexact
```

The tuple must have exactly 2 elements, and the second must be a Python `bool`.

### Argument Exactness

If your function needs to know whether its arguments are exact, set `arg_type = "exact"`:

```
def lowindex(x):
    '''Returns the lower 16 bits, preserving exactness'''
    return (int(x[0]) % 65536, x[1])

lowindex.arg_type = "exact"
```

When `arg_type = "exact"` is set, **all** arguments are passed as 2-tuples `(value, is_exact)`.

### Combining Argument and Return Exactness

```
def add_exact(a, b):
    '''Add two numbers, exact only if both inputs are exact'''
    return (a[0] + b[0], a[1] and b[1])

add_exact.arg_type = "exact"
```

### Error Handling

If `arg_type` is set to an unrecognized value, specs reports an error at initialization:

```
def bad_function(x):
    pass

bad_function.arg_type = "bogus"  # ERROR at startup
```

If a function with `arg_type = "exact"` tries to use an argument as a plain value (not as a tuple), Python raises a `TypeError`, which **specs** reports.

---

# Chapter 15: Practical Examples

This chapter presents worked examples that combine multiple features. Each example shows a realistic problem and a complete specification that solves it.

## Example 1: Reformatting ls -l Output

**Problem**: Given `ls -l` output, extract just the filename (last word's last component after `/`) and right-align it in a 40-character field, followed by the file size (5th word, right-aligned in 10 characters).

```
ls -l | specs substr fs / field -1 of word -1 1.40 right   w5 nw
```

This uses a `SUBSTRING` to extract the filename by treating `/` as the field separator within the last word.

## Example 2: Summing a Column of Numbers

**Problem**: Given a file with one number per line, print each number and a running total, then print the grand total.

```
# summing.spec
   a: word 1   1
      set "#0+=a"
      print "#0" nextword
   EOF
      /Grand total:/ 1
      print #0 strip  nextword
```

Run: `cat numbers.txt | specs -f summing.spec`

## Example 3: Department Break Report

**Problem**: Process a CSV file with `department,firstname,lastname` records. Print employees grouped by department, showing the department name only when it changes.

```
# dept_report.spec
FIELDSEPARATOR ,
c: FIELD 1   .
   FIELD 3   10
   /,/        NEXT
   FIELD 2   NEXTWORD
BREAK c
    ID c 1
```

Run: `sort -t, -k1 employees.csv | specs -f dept_report.spec`

Output:
```
Finance  Cheung, Wiley
         Cockett, Leonard
         Lugo, Dorie
Payroll  Eck, Eddy
         Polen, Janel
R&D      Driskell, Shawnna
...
```

## Example 4: Parsing git log

**Problem**: Parse `git log` output to extract one line per commit: hash, author username, and date as seconds since epoch.

```
# gitlog.spec
IF "first()" THEN
    SET "#4:=word(2)"
ELSE
    PRINT "#4"                1
    WORD 2             NEXTWORD
    READ
    WORD 2-6 tf2s "%c" NEXTWORD
    WHILE "word(1)!='commit'" DO
        READSTOP
    DONE
    SET "#4:=word(2)"
ENDIF
```

Run: `git log | specs -f gitlog.spec`

Output:
```
df3438ed9e95c2aa37a429ab07f0956164ec4229 synp71 1548013241.000000
e6d7f9ac591379d653a5685f9d75deccc1792545 synp71 1548011387.000000
```

## Example 5: Context-Aware Difference Report

**Problem**: Given a file of daily temperatures, print each day's temperature and the change from the previous day.

```
# temps.spec
t: w1 1
IF "first()" THEN
    /n/a/ nw
ELSE
    CONTEXT -1
    p: w1 .
    CONTEXT 0
    PRINT "t - p" nw
ENDIF
```

Run: `cat temps.txt | specs -f temps.spec`

On input `20`, `23`, `19`, `25`:
```
20 n/a
23 3
19 -4
25 6
```

## Example 6: Word Frequency Counter

**Problem**: Count how many times each word appears in a file, then print the top 10 by frequency.

```
# wordfreq.spec
a: w1 .
   IF "!eof()" THEN
       NOWRITE
   ENDIF
EOF
   PRINT "fmap_dump(a,'txt','cd',1)" 1
```

Run: `tr '[:space:]' '\n' < file.txt | grep -v '^$' | specs -f wordfreq.spec | head -10`

The `fmap_dump` with sort order `cd` (count descending) gives the most frequent words first.

## Example 7: Estimating PI (Monte Carlo Method)

**Problem**: Estimate pi using random points in a unit square.

```
# pi_estimate.spec
   WORD 1 .                       # consume the input record
   SET  "#0:=2*(rand()-0.5)"      # random x in [-1,1]  (rand() returns float when no arg)
   SET  "#1:=2*(rand()-0.5)"      # random y in [-1,1]
   SET  "#2:=pow(#0,2)+pow(#1,2)" # squared distance from origin
   SET  "#4:=#4+1"                # total points
   IF   "#2<=1.0" THEN
       SET  "#3:=#3+1"            # points inside unit circle
   ENDIF
EOF
   PRINT  "4*#3/#4" 1             # pi estimate
```

Run: `seq 100000 | specs -f pi_estimate.spec`

For 100,000 iterations, you get about 3–4 significant digits of pi.

## Example 8: Processing Web Server Logs

**Problem**: Parse Apache combined log format, extract the URL and HTTP status code, filter for 404s, and count the 10 most common 404 URLs.

```
# log404.spec
   fs " "
   a: f7  .                  # URL field (7th word)
   b: f9  .                  # status code field
   IF "b == 404" THEN
       NOWRITE
   ELSE
       CONTINUE
   ENDIF
EOF
   PRINT "fmap_dump(a,'txt','cd',0)" 1
```

Run: `cat access.log | specs -f log404.spec`

## Example 9: Generating Shell Commands

**Problem**: Given a list of filenames in a text file, generate `gzip` commands for each.

```
cat filelist.txt | specs --shell /gzip -9/ 1 w1 nw
```

The `--shell` flag executes each output line as a shell command. Remove `--shell` first to verify the commands look correct.

## Example 10: Fixed-Width Report with Ellipsis

**Problem**: Generate a report with a name column of exactly 30 characters, truncated with ellipsis in the middle if too long.

```
# report.spec
FIELDSEPARATOR ,
w1 (1, 30, "R3")    # name: centered ellipsis if too long
f2 nw               # second field: value
```

A name like `Supercalifragilisticexpialidocious` becomes `Supercali...docious` (30 characters).

---

```{=latex}
\appendix
```

# Appendix A: Decision Guide

## Where Should I Write My Spec?

```
Is the spec short enough to type on one line?
├─ Yes, and it's a one-off task → Use the command line
└─ No, or you'll reuse it → Write a spec file (-f)
```

## Should I Use a Configuration File Entry or a Command-Line -s Flag?

```
Is the value used across many different specs?
├─ Yes → Put it in ~/.specs
└─ No, it's specific to this invocation → Use --set name=value or -s name=value
```

## Should I Use READ/READSTOP or CONTEXT?

| Scenario | Use |
|----------|-----|
| Looking at the record immediately ahead *without* consuming it | `CONTEXT 1` |
| Looking at any number of records ahead or behind | `CONTEXT n` |
| Consuming multiple records to assemble one output record | `READ` / `READSTOP` |
| Consumed one record too many in a loop | `UNREAD` |
| Processing variable-length blocks from a structured format | `READ` + `WHILE` + `UNREAD` |

## Should I Write a Python Function?

Write a Python function when:

- You need an algorithm that's complex to express in the ALU (nested loops, data structures, recursion)
- You want to use a Python library (`re`, `requests`, `json`, `csv`, etc.)
- You need stateful computation more complex than a counter (dict accumulation, complex objects)
- You want reusable code with Python's testing tools

Use a built-in function or the ALU when:

- The operation is string manipulation, arithmetic, or time conversion
- Performance is critical (built-in functions are faster than Python calls)
- The Python runtime may not be available in the deployment environment

## Should I Use eof() or the EOF Keyword?

Both trigger the run-out cycle. Choose based on readability:

- `EOF` keyword: When the run-out code is clearly separate from per-record code (especially in spec files where indentation helps)
- `eof()` in an `IF`: When the run-out logic is interleaved with per-record logic or when a single conditional covers both

## When Should I Use Counters (#n) vs. Field Identifiers (a:)?

| Feature | Counters (#0, #1, ...) | Field Identifiers (a:, b:, ...) |
|---------|----------------------|--------------------------------|
| Persist across records | Yes | Yes (via stats functions) |
| Can be incremented | Yes (+=) | No (captured fresh each record) |
| Can be used in break detection | No | Yes (BREAK a) |
| Available in statistical functions | No | Yes (sum(a), average(a), etc.) |
| Suitable for accumulators | Yes | With fmap_sample() |

## When Should I Use --threaded?

**Deprecated** — threaded mode may be removed in a future release. Avoid relying on it in new work.

Use `--threaded` when:

- Processing large files (millions of records)
- *Processing* is the bottleneck (CPU-intensive specifications, complex ALU expressions), so that reads and writes can proceed while the processor works

Threaded mode will **not** help when the I/O itself is the bottleneck (slow disks, network filesystems). In that case the processor is merely waiting for data, and giving the reader and writer their own threads gives it nothing extra to do. Most practical specifications fall into exactly this category — they are I/O-bound rather than CPU-bound — which is why threaded mode is deprecated and may be removed in a future release.

Do NOT use `--threaded` with:

- Rolling context (`CONTEXT`)
- Multiple input streams

---

# Appendix B: Quick Reference  {#appendixb}

## Command-Line Switches

| Switch | Short | Description |
|--------|-------|-------------|
| `--inFile file` | `-i` | Read input from file |
| `--outFile file` | `-o` | Write output to file |
| `--specFile file` | `-f` | Read spec from file |
| `--config file` | `-c` | Use alternate config file |
| `--set name=val` | `-s` | Set configured literal |
| `--inCmd cmd` | `-C` | Use command output as input |
| `--shell` | `-X` | Execute output lines as shell commands |
| `--threaded` | `-t` | Run in threaded mode (deprecated) |
| `--verbose` | `-v` | Verbose error output |
| `--stats` | | Print runtime statistics |
| `--progress` | | Show progress on stderr |
| `--is2` – `--is8` | | Additional input streams |
| `--os2` – `--os8` | | Additional output streams |
| `--recfm format` | | Input record format (D/F/FD) |
| `--lrecl n` | | Record length for fixed formats |
| `--linedel char` | | Line delimiter character |
| `--spaceWS` | `-w` | Space-only word separator |
| `--timezone name` | | Set timezone |
| `--regexType list` | | Set regex grammar |
| `--pythonFuncs val` | | on/off/auto |
| `--pythonErr val` | | throw/NaN/zero/nullstr |
| `--no-while-guard` | | Disable while-guard (default limit: 5000) |
| `--toASCII` | | Convert non-ASCII to period |
| `--force-read-input` | | Read input even if unused |
| `--EXP-UTF8` | | Experimental UTF-8 character counting |
| `--debug-alu-comp` | | (Debug builds) Print ALU compile info |
| `--debug-alu-run` | | (Debug builds) Print ALU eval info |
| `--help topic` | | Print help |
| `--info` | | Print build information |

\newpage
## Special Configured Literals

These are keys that, when set in `~/.specs` (or via `-s`), change **specs** behavior rather than simply becoming an `@name` literal. See [Chapter 4](#configured-literal-settings) for details.

| Key | Default | Effect |
|-----|---------|--------|
| `timezone` | System timezone | Timezone used by date/time conversion functions |
| `locale` | System locale | Locale used by number-formatting functions like `pretty()`; `global` resets to the OS locale |
| `regexType` | `ECMAScript` | Regex grammar/flags used by `rmatch()`, `rsearch()`, `rreplace()` |
| `SPECSPATH` | `$HOME/specs` or `%APPDATA%\specs` | Search path for spec files and Python function files |
| `pythonDisable` | unset | Set to `1` to permanently disable Python function loading |
| `NO_WARN_REDEFINED_FID` | unset | Set to suppress the "Field Identifier redefined" warning |
| `EmptyFrequencyMapMessage` | `""` | String returned by `fmap_dump()` for an empty frequency map |
| `while-guard-limit` | `5000` | Maximum `WHILE` loop iterations before specs aborts |

## Input Record Formats (--recfm)

| Format | Nickname | lrecl | linedel | Notes |
|--------|----------|-------|---------|-------|
| `D` | delimited | n/a | optional | Default. Records end at OS line separator |
| `F` | fixed | required | n/a | Exactly lrecl characters per record |
| `FD` | fixed-delimited | required | optional | Fixed length with line delimiter |

## Output Placement Syntax

| Syntax | Meaning |
|--------|---------|
| `n` | Absolute column n |
| `n-m` | Range n to m (truncates/pads) |
| `n.len` | len characters starting at column n |
| `n` or `NEXT` | Immediately after previous output |
| `nw` or `NEXTWORD` | After one space |
| `nf` or `NEXTFIELD` | After one tab |
| `n.W` / `nw.W` / `nf.W` | Relative placement with fixed width W |
| `.` | No output (used with field identifiers) |
| `(start)` | Composed: dynamic start column |
| `(start, width)` | Composed: dynamic start and width |
| `(start, width, align)` | Composed: dynamic alignment |

\newpage
## Input Source Syntax

| Syntax | Example | Meaning |
|--------|---------|---------|
| `n` | `5` | Character at position 5 |
| `n:m` | `3:7` | Characters 3 to 7 |
| `n.len` | `5.8` | 8 characters starting at 5 |
| `-n` | `-1` | Last character |
| `1-*` | `1-*` | Entire record |
| `wn` / `word n` | `w2` | n-th word |
| `wn:m` / `word n:m` | `w1:3` | Words n through m |
| `w-1` | `w-1` | Last word |
| `fn` / `field n` | `f3` | n-th field |
| `fn:m` / `field n:m` | `f2:4` | Fields n through m |
| `SUBSTR ... OF ...` | | Substring of another source |
| `PRINT "expr"` / `? "expr"` | | ALU expression result |
| `ID fid` | `ID a` | Value of field identifier |
| `/text/` | `/hello/` | String literal |
| `xHH...` | `x4142` | Hex literal: binary string from hex pairs |
| `RECNO` / `NUMBER` | | Record counter (10 digits) |
| `TODclock` | | Seconds since epoch (start of run) |
| `DTODclock` | | Seconds since epoch (current record) |
| `TIMEDIFF` | | Microseconds since start of run |

## Alignment

| Keyword | Effect |
|---------|--------|
| `left` | Left-align (default) |
| `right` | Right-align |
| `center` / `centre` | Center |

## Conversions

The table below holds conversions used within **Data Fields**. Where you see `fmt` this is a string representing a time format as in the function **[strftime](https://man7.org/linux/man-pages/man3/strftime.3.html)**, with the addition of the `%`*x*`f`, where *x* is between zero and 6, which represents fractional seconds. For example, `%H:%M:%S.%3f` may yield `15:02:37.372`.

| Conversion | Effect |
|------------|--------|
| `ucase` | Uppercase |
| `lcase` | Lowercase |
| `rot13` | ROT-13 |
| `BSWAP` | Reverse bytes |
| `C2B` | Characters to binary |
| `C2X` | Characters to hex |
| `B2C` | Binary to characters |
| `X2CH` | Hex to characters |
| `b2x` | Binary data to hex |
| `D2X` | Decimal to hex |
| `X2D` | Hex to decimal |
| `STRIP` | Remove leading/trailing whitespace |
| `ti2f fmt` | Internal time (µs) to string |
| `tf2i fmt` | String to internal time |
| `s2tf fmt` | Seconds to string |
| `tf2s fmt` | String to seconds |
| `mcs2tf fmt` | Microseconds to string |
| `tf2mcs fmt` | String to microseconds |

## Special Spec Units

| Keyword | Description |
|---------|-------------|
| `WRITE` | Emit current output record and reset buffer |
| `NOWRITE` / `NOPRINT` | Suppress automatic output for this cycle |
| `READ` | Read next input record (empty if at end) |
| `READSTOP` | Read next input record (stop if at end) |
| `UNREAD` | Push current record back for next cycle |
| `REDO` | Make current output the new input |
| `SPLITW` | Split by words into separate output records |
| `SPLITF` | Split by fields into separate output records |
| `SELECT n` | Switch to input stream n |
| `SELECT FIRST` / `SELECT SECOND` | Switch between primary and secondary input |
| `OUTSTREAM n` | Switch output to stream n |
| `OUTSTREAM STDERR` | Switch output to stderr |
| `CONTEXT n` | Change active input to record at offset n |
| `EOF` | Marks units that run only in the run-out cycle |
| `CONTINUE` | Skip to next cycle immediately |
| `ABEND message` | Abort with error message |
| `ASSERT condition` | Abort if condition is false |
| `SKIP-WHILE cond` | Skip while condition is true (one-time) |
| `SKIP-UNTIL cond` | Skip until condition is true (one-time) |
| `BREAK fid` | Execute following units only when break level fid is established |
| `SET "expr"` | Evaluate expression (assignment) |
| `REQUIRES name` | Abort if configured literal name is not defined |
| `PRINTONLY` | Suppress per-record output by default |
| `KEEP` | Don't clear output buffer for non-printed records |
| `STOP ALLEOF` | Stop when all input streams exhausted (default) |
| `STOP ANYEOF` | Stop when any input stream exhausted |
| `STOP n` | Stop when stream n is exhausted |
| `WORDSEPARATOR str` or `WS str` | Set word separator characters |
| `FIELDSEPARATOR str` or `FS str` | Set field separator characters |
| `PAD char` | Set the padding character (default: space) |

## ALU Operators

| Op | Name | Notes |
|----|------|-------|
| `+` | Add / Unary plus | |
| `-` | Subtract / Unary minus | |
| `*` | Multiply | |
| `/` | Divide | Returns quotient |
| `//` | Integer divide | |
| `%` | Remainder | |
| `||` | Concatenate | |
| `<` `<=` `>` `>=` | Numeric compare | |
| `=` `!=` | Smart equality | Numeric if both numeric |
| `==` `!==` | Strict equality | Always string |
| `<<` `<<=` `>>` `>>=` | String compare | Alphabetical |
| `!` | Logical NOT | |
| `&` | Logical AND | |
| `|` | Logical OR | |

## Assignment Operators (in SET)

| Op | Effect |
|----|--------|
| `:=` | Assign |
| `+=` | Add |
| `-=` | Subtract |
| `*=` | Multiply |
| `/=` | Divide |
| `//=` | Integer divide |
| `%=` | Remainder |
| `||=` | Append string |

---

# Appendix C: Installation {#appendixc}

There are two ways to get **specs** running on your machine: installing a pre-built binary package, or building it locally from source. Installing a pre-built package is the fastest route for most people; building from source is useful if no package exists for your platform, if you need a different Python version than the one bundled with the official packages, or if you want to hack on **specs** itself.

## Installing from Binaries

Every release of **specs** publishes pre-built packages for Linux (RPM and DEB), macOS (PKG), and Windows (MSI and a standalone EXE).

### Finding the right release

1. Go to the [GitHub releases page](https://github.com/yoavnir/specs2016/releases).
2. GitHub lists releases newest-first, and the most recent one is tagged **Latest** with a green badge next to its version number, near the top of the page. There may be newer ones, but they will either be marked as **Pre-release* or not marked at all. Make sure you pick the latest release, or the specific pre-relase or older version that you intend to use.
3. Scroll down to your release's **Assets** section, which lists every downloadable file for that release.

![The GitHub releases page, with the most recent release marked with a "Latest" badge](XXDOCS/resources/releases_page.png)

### Choosing the right package for your operating system

The assets section of a release contains several files. Pick the one that matches your operating system and, where relevant, your CPU architecture:

| File pattern | Operating system | Notes |
|---|---|---|
| `specs-<version>-1.x86_64.rpm` | Linux, RPM-based (Fedora, RHEL, CentOS, openSUSE, ...) | Built for 64-bit Intel/AMD (`x86_64`) machines |
| `specs_<version>_amd64.deb` | Linux, DEB-based (Ubuntu, Debian, Mint, ...) | 64-bit Intel/AMD |
| `specs_<version>_arm64.deb` | Linux, DEB-based, ARM64 | e.g. Raspberry Pi (64-bit OS), other ARM-based PCs, or Linux running under Parallels on an Apple Silicon Mac |
| `specs-<version>.pkg` | macOS | Apple Silicon package, bundled with Python 3.12 |
| `specs-<version>.msi` | Windows | Installer, no Python support |
| `specs-<version>-python312.msi` | Windows | Installer, bundled with Python 3.12 |
| `specs-<version>-windows-x64.exe` | Windows | Standalone executable, no installer, no Python support |
| `specs-<version>-python312-windows-x64.exe` | Windows | Standalone executable, no installer; requires Python 3.12 (`python312.dll`) on the PATH |

If you're not sure which Linux package family your distribution uses, check with your package manager: `dnf`, `yum`, or `zypper` distributions use RPM; `apt` or `apt-get` distributions use DEB. If you're not sure of your CPU architecture on Linux, run `uname -m`; `x86_64` means Intel/AMD 64-bit, while `aarch64` means ARM64.

### Installing the package

**Linux (RPM-based):**
```
sudo dnf install ./specs-1.0.0-1.x86_64.rpm
```
or, without a dependency-resolving package manager:
```
sudo rpm -i specs-1.0.0-1.x86_64.rpm
```

**Linux (DEB-based):**
```
sudo apt install ./specs_1.0.0_amd64.deb
```
or:
```
sudo dpkg -i specs_1.0.0_amd64.deb
```

**macOS:**

Double-click the downloaded `.pkg` file and follow the installer prompts, or from the command line:
```
sudo installer -pkg specs-1.0.0.pkg -target /
```
Recent versions of macOS are strict about where packages come from and may refuse to run the installer, complaining that it is from an "unidentified developer" or quarantining it. If that happens, clear the quarantine flag before installing:
```
xattr -dr com.apple.quarantine /path/to/specs-1.0.0.pkg
```

**Windows (MSI):**

Double-click the downloaded `.msi` file and follow the installer prompts. This adds `specs.exe` to your PATH. Choose `specs-<version>-python312.msi` if you want Python integration; otherwise use `specs-<version>.msi`.

**Windows (standalone EXE):**

Download `specs-<version>-windows-x64.exe` (or the `-python312-` variant for Python support), rename it to `specs.exe` if you like, and place it in a directory that's on your PATH. Unlike the MSI, this is just a single file with no installer and no automatic PATH update. If you download the Python-enabled variant, you need Python 3.12 installed (so that `python312.dll` is available) since, unlike the MSI, it does not bundle its own copy.

Note: although Windows for ARM64 is not officially supported, the x64 packages run fine on it under emulation. For Python integration on ARM64, install the x64 build of Python 3.12.

\newpage
### Verifying the installation

Once installed, open a new terminal (so that PATH changes take effect) and run:
```
specs @version
```
This should print the version string, for example `1.0.0`. To check the platform and whether Python support is compiled in, run:
```
specs @platform
```
which prints something like:
```
POSIX (darwin) system using the g++ compiler and Python 3.9.6 - release variation
```
For full build provenance, run:
```
specs @build-info
```
which prints something like:
```
Built on GitHub (id 27938338680; build 262) from commit 3a14b4f of version 1.0.0-beta at 2026-06-22T08:05:28 UTC
```

## Building From Source

If no pre-built package fits your needs — for example, you need a different Python version, or you're contributing to **specs** — you can build it yourself. This is the same procedure documented in `BUILDING.md` at the root of the repository.

### Getting the sources

Download your copy of **specs** from [GitHub](https://github.com/yoavnir/specs2016) in either of two ways:

1. Using git: `git clone https://github.com/yoavnir/specs2016.git`
2. Using http: `wget https://github.com/yoavnir/specs2016/archive/dev.zip`

### Prerequisites

* A C++17-compatible compiler (GCC, Clang, or MSVC)
* Python 3 runtime if you are using `GCC` or `CLang`
* Python 3 development environment (optional, for Python integration support)
  * On Linux: the `python3-devel` (or `python3-dev`) package that matches your Python version
  * On Mac OS: the Xcode command-line tools (which include Python headers)
  * On Windows: a standard Python 3 installation includes the required headers and libraries

### Checking out a stable version

If you have downloaded a git repository, first make sure to check out a stable tag such as v0.9.9:
```
git checkout v0.9.9
```
A good way to get the latest stable release is to check out the `stable` branch and rebase to its tip:
```
git checkout stable
git rebase
```

### Building on Linux and Mac OS (make)

Change to the `specs/src` directory, and run the following commands:

1. `python setup.py` -- use `python3` or `python3.x` if your default Python version is 2.7
2. `make -j 8 ci` -- equivalent to the targets `clean`, `all`, and `run_tests`.
3. `sudo make install`

The `setup.py` script auto-detects your compiler, Python installation, and platform capabilities. It generates a `Makefile` tailored to your environment.

**Python support**

Python support is detected automatically by `setup.py`. To explicitly control it:

* `python setup.py --python python3.11` -- use a specific Python version
* `python setup.py --python no` -- disable Python support entirely

Only Python 3 is supported. To enable Python support, you need the `python3-devel` package (or equivalent) that matches your Python version installed.

**Notes**

* On some Mac machines, `sudo make install` will cause a warning about being the wrong user.
* You can pass `-v DEBUG` to `setup.py` to build a debug version.
* You can pass `-v PROF` to `setup.py` to build a release version with symbols, useful for profiling.
* You can pass `--static` to `setup.py` to statically link libstdc++ (useful for portable binaries).

### Building on Windows with MSBuild

Start from the repository root directory (do **not** change to `specs/src`).

**Without Python support (default)**
```
msbuild specs\specs.sln /p:Configuration=Release /p:Platform=x64
```

**With Python support**

To build with Python support, add `/p:EnablePython=true` to the command line. Python 3 and its development files must be installed on the build machine:
```
msbuild specs\specs.sln /p:Configuration=Release /p:Platform=x64 /p:EnablePython=true
```

Python is auto-detected from the system PATH. If Python is not in your PATH or you want to use a specific installation, provide the installation directory explicitly:
```
msbuild specs\specs.sln /p:Configuration=Release /p:Platform=x64 /p:EnablePython=true 
     /p:PythonDir=C:\Python312
```

You may also override the detected version numbers if needed:
```
msbuild specs\specs.sln /p:Configuration=Release /p:Platform=x64 /p:EnablePython=true 
     /p:PythonDir=C:\Python312 /p:PythonVerNoDot=312 /p:PythonFullVer=3.12.0
```

**After building**

Copy the resulting `specs.exe` from the `specs\bin\Release\` directory to a location in your PATH.

**Notes**

* With Python support enabled, the appropriate Python DLL (e.g., `python312.dll`) must be in the PATH at runtime.
* To build the Debug configuration, replace `Release` with `Debug` in the commands above.

### Building on Windows with make

As an alternative to MSBuild, you can use `make` on Windows. Change to the `specs/src` directory and run:

1. `python setup.py -c VS`
2. `make some`

This approach uses the Visual Studio `cl.exe` compiler via `make` and supports the same `--python` flag as on other platforms.

### Known Issues

* Regular expression grammars other than the default `ECMAScript` don't work except on Mac OS.
* On Windows with Python support, the appropriate DLL (like `python312.dll`) must be in the PATH.

*Note:* Although Windows for ARM64 is not officially supported, that platform will run the x64 version just fine. For Python integration, you'll need to install the x64 version of Python.

### Verifying the build

Whichever platform you built on, verify with:
```
specs @version
specs @platform
```
as described above under "Verifying the installation."

---

*This guidebook covers specs version XXVERSION. The specs project is hosted at [https://github.com/yoavnir/specs2016](https://github.com/yoavnir/specs2016).*
