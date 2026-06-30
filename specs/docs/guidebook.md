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
numbersections: false
keywords: 
  - "specs"
  - "guidebook"
  - "AI-generated"
date: "XXDATE"
header-includes: |
  \usepackage{etoolbox}
  \pretocmd{\chapter}{\clearpage}{}{}
include-before: |
  \begin{titlepage}
  \centering
  {\Huge\bfseries specs\par}
  \vspace{2cm}
  {\Large The complete guidebook\par}
  \vspace{4cm}
  {\textit\underline\color{blue}https://github.com/yoavnir/specs2016\par}
  \vfill
  {\Large Version: XXVERSION\par}
  \vspace{1cm}
  {\large XXDATE\par}
  \end{titlepage}
  \clearpage
  \chapter*{Preface}
  \textit{A complete tutorial and reference for the} \textbf{specs} \textit{text-processing utility}
  \bigskip\par
  This guidebook teaches you \textbf{specs} from the ground up. It assumes no prior knowledge of the tool, though familiarity with a Unix command line is helpful. By the end you will be able to write specifications ranging from one-liners that reformat a column of numbers to multi-page programs that join files, compute statistics, and call Python functions.
  \bigskip\par
  \textbf{How to use this book}
  \begin{itemize}
  \item Read Chapters 1--5 to understand the mental model and learn the three places where you can give specs its instructions (the command line, the configuration file, and spec files).
  \item Read Chapters 6--13 to master every feature of the language.
  \item Read Chapter 14 when you need to extend specs with Python.
  \item Use Chapter 15 for inspiration, and Appendices A--B as a desk reference.
  \end{itemize}
  Every example in this book can be copied and run as-is. All examples assume a POSIX shell (bash). Where shell-quoting matters it is called out explicitly.
  \clearpage
---

# Chapter 1: Introduction and Mental Model

## What is specs?

**specs** is a command-line utility for parsing and re-arranging text. Its name comes from "specifications" — you describe *what you want done* rather than *how to do it* imperatively. Think of it as an infinitely configurable version of `awk` or `cut`, one that also handles multi-record aggregation, time conversion, regular expressions, statistics, and arithmetic.

**specs** was originally a stage in the **CMS Pipelines** system on IBM mainframes running VM/ESA or z/VM. This version is a modern re-implementation for Linux, macOS, and Windows, liberally extended with new features, and with many of the mainframisms replaced with UNIX-isms. As an example, REXX integration was replaced with Python integration.

### What problems does specs solve?

Here are the kinds of questions specs was built to answer:

- *"I have a CSV file. Give me columns 2 and 5, tab-separated, in a fixed-width column."*
- *"Parse this web-server log and give me just the URL and HTTP status code."*
- *"Read all these numbers and print their sum at the end."*
- *"I have two files with matching row counts — join the third column of each."*
- *"Look at adjacent records and flag any two consecutive entries that differ by more than 10."*
- *"Run this command for every line in the file."*

### Installation

Binary packages for the latest release are available on the [GitHub releases page](https://github.com/yoavnir/specs2016/releases). Download the package for your operating system and install it.

To build from source, see `BUILDING.md` in the project root.

After installation, verify it works:

```
specs @version
```

This should print the version string, for example `1.0.0`. Alternatively, you can get even more build into:

```
$ specs @build-info
Built on github (id 27938338680; build 262) from commit 3a14b4f of version 1.0.0-beta at 2026-06-22T08:05:28 UTC
```

## The Mental Model

Understanding three concepts unlocks everything else:

### Records

**specs** is a record-oriented processor. It reads the input one *record* at a time (normally one line at a time), runs your specification against that record, and emits zero or more output records. This cycle repeats until the input is exhausted. For most purposes a record is simply one line of text, but specs can also handle fixed-length binary records and streams delimited by characters other than newline — see "Record Formats" in Chapter 6.

```
Input stream          specs                Output stream
─────────────         ──────────────────   ─────────────
line 1           →    [specification] →    result 1
line 2           →    [specification] →    result 2
line 3           →    [specification] →    result 3
…                →    [specification] →    …
```

This default one-in-one-out pattern can be broken: you can emit multiple output records per input record, read multiple input records per cycle, or suppress output entirely. These capabilities are covered in Chapter 12.

### The Specification

The *specification* is the set of instructions you give specs. It describes what to extract from the input record, how to transform it, and where to place it in the output record.

A specification consists of **spec units** — small building blocks that each perform one action. The most common spec unit is a **data field**, which copies a piece of the input to a piece of the output.

### Where Specs Lives in a Pipeline

**specs** is designed to sit in a shell pipeline:

```
some-command | specs [switches] [spec-units] | next-command
```

Input comes from **standard input** (or a file with `-i`). Output goes to **standard output** (or a file with `-o`). This makes specs a natural glue between other Unix tools.

## Three Places to Give Instructions

You will give specs its instructions in three different places, depending on the situation:

| Where | When to use it |
|-------|---------------|
| **Command-line arguments** | Short, one-off tasks. The spec fits in a line or two. |
| **Configuration file** (`~/.specs`) | Values that stay the same across many invocations: timezone, locale, personal constants, Python toggle. |
| **Spec file** (`-f filename`) | Longer specifications; when you want comments, indentation, and reuse. |

You will learn all three. A quick decision guide is in Appendix A; details are in Chapters 3, 4, and 5.

---

# Chapter 2: Your First Specifications

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

## Spec Units and Data Fields

A **data field** is the workhorse spec unit. Its full form is:

```
[fieldIdentifier:] InputSource [STRIP] [conversion] OutputPlacement [alignment]
```

Most parts are optional. At minimum you need an `InputSource` and an `OutputPlacement`.

### Your First Data Field

```
echo "Hello, world" | specs 1-5 1
```

Output:
```
Hello
```

`1-5` selects characters 1 through 5; `1` places them at column 1 of the output.

### Selecting by Position

Character ranges use 1-based indexing:

| Syntax | Meaning |
|--------|---------|
| `5` | Character at position 5 |
| `3-7` | Characters 3 through 7 inclusive |
| `5.8` | 8 characters starting at position 5 |
| `-1` | The last character |
| `-3` | The third character from the end |
| `1-*` | The entire record |

Let's try a few:

```
echo "ABCDEFGH" | specs 3-5 1
```
Output: `CDE`

```
echo "ABCDEFGH" | specs 1.3 1
```
Output: `ABC` (3 characters starting at 1)

```
echo "ABCDEFGH" | specs -3-* 1
```
Output: `FGH` (from the third-to-last character to the end)

### Selecting Words

A **word** is a sequence of non-whitespace characters. Words are separated by one or more whitespace characters (the default word separator).

| Syntax | Meaning |
|--------|---------|
| `w1` or `word 1` | First word |
| `w3` or `word 3` | Third word |
| `w1-3` or `words 1-3` | Words 1 through 3 |
| `w-1` or `word -1` | Last word |
| `w2.3` | Three words starting at the second word |

```
echo "the quick brown fox" | specs w2 1
```
Output: `quick`

```
echo "the quick brown fox" | specs w1-2 1
```
Output: `the quick`

```
echo "the quick brown fox" | specs w-1 1
```
Output: `fox`

### Selecting Fields

A **field** is similar to a word, but fields are separated by *exactly one* field separator character (a tab by default). This means empty fields are possible.

| Syntax | Meaning |
|--------|---------|
| `f1` or `field 1` | First field |
| `f3` | Third field |
| `f1-3` or `fields 1-3` | Fields 1 through 3 |
| `f-1` or `field -1` | Last field |

Words vs. fields are explained in detail in Chapter 6. The key difference: consecutive words can be separated by multiple separators, while consecutive fields are separated by exactly one separator (so empty fields are possible with fields).

### String Literals as Input

You can place literal text in the output by using a string literal as the input source. Delimiters can be `/`, `'`, or `"`:

```
echo "test" | specs /Hello/ 1
```
Output: `Hello`

On the command line, the most common delimiters are `/` (slashes) or unquoted text that does not look like a keyword. To include special shell characters, wrap the entire argument in double quotes.

### Multiple Data Fields

A specification can have multiple data fields. They are processed in order, and each can write to a different output position:

```
echo "Alice 42" | specs w1 1 w2 10
```
Output: `Alice    42` (name at column 1, age at column 10)

### Relative Output Placement

Instead of specifying absolute column numbers, you can use relative placement:

| Keyword | Meaning |
|---------|---------|
| `n` or `next` | Immediately after the previous output |
| `nw` or `nextword` | After a space following the previous output |
| `nf` or `nextfield` | After a tab following the previous output |

```
echo "Alice 42" | specs /Name:/ 1 w1 nextword /Age:/ nextword w2 nextword
```
Output: `Name: Alice Age: 42`

Note that `nextword` adds a single space before the next piece of output.

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
2024-01-15   ERROR        192.168.1.100
```

---

# Chapter 3: When to Use Command-Line Flags

Command-line flags (switches) modify how specs behaves. They come *before* the spec units on the command line. Most simple invocations don't need any flags — the defaults cover the common case.

Here is a guided tour of when each flag is useful.

## Input and Output

### `-i filename` / `--inFile filename`

Read input from a file instead of stdin. Use this when you're not piping from another command:

```
specs 1-* 1 -i mydata.txt
```

### `-o filename` / `--outFile filename`

Write output to a file instead of stdout. Useful when the output is large or when you want to avoid it appearing on screen:

```
cat input.txt | specs w2 1 -o output.txt
```

### `-C cmd` / `--inCmd cmd`

Use the output of a shell command as the input stream:

```
specs --inCmd "ls -l /tmp" w9 1
```

This avoids a pipe when you want the input to be the output of a command.

### `--is2` through `--is8` / `--os2` through `--os8`

Assign files to additional input or output streams. This is an advanced feature covered in Chapter 12.

### `--recfm format`, `--lrecl n`, `--linedel char`

Control how input records are structured. The default (`D` for delimited) reads one line at a time. Use `F` for fixed-length records (requires `--lrecl`) or `FD` for fixed-length lines. Full details and examples are in the "Record Formats" section of Chapter 6; a quick reference table is in Appendix B.

## Controlling Output

### `-X` / `--shell`

Execute each output record as a shell command rather than printing it. This is a powerful but dangerous flag — make sure your specification is correct before using it! A typical use is generating a series of shell commands:

```
ls *.log | specs /rm/ 1 w1 nextword --shell
```

This would delete all `.log` files. (Use without `--shell` first to verify the commands look right.)

### `--toASCII`

Convert non-ASCII characters in the output to periods. Useful when downstream tools can't handle Unicode.

### `--progress`

Print a counter to stderr every second showing how many records have been processed. Useful for long-running jobs on large files.

## Debugging and Diagnostics

### `-v` / `--verbose`

Print extra information when something goes wrong. When rolling context is in use (Chapter 13), it also reports the buffer sizes:

```
specs: Using a 3-record rolling context: 2 records forward and 1 records backward.
```

Use `-v` as your first step when a specification produces unexpected output.

### `--stats`

Print runtime statistics at the end of the run: record counts, wall-clock time, and CPU time. Useful for performance tuning:

```
specs 1-* 1 --stats < large_file.txt > /dev/null
```

## Behavior Modifiers

### `-f filename` / `--specFile filename`

Read the specification from a file rather than the command line. This is covered in depth in Chapter 5.

### `-c filename` / `--config filename`

Use a different configuration file instead of `~/.specs`. Useful for testing or when managing multiple configurations:

```
specs -c ~/specs-project1 -f myspec.txt
```

### `-s name=value` / `--set name=value`

Set a configured literal (see Chapter 4) from the command line. Lets you parameterize a spec file:

```
specs -f report.spec -s threshold=100
```

### `-t` / `--threaded`

Run in threaded mode: separate threads for the reader, the processor, and the writer. This can improve throughput on large files when I/O is the bottleneck. The default since version 0.9.5 is single-threaded.

### `--spaceWS` / `-w`

Treat only the space character as a word separator, rather than all locale-defined whitespace. By default, tabs and other whitespace characters also separate words.

### `--no-while-guard`

Disable the while-guard, which normally causes specs to abort after 5000 iterations of a `WHILE` loop (a safety net against infinite loops). Use this when you intentionally have a long-running loop. The default limit of 5000 can also be changed via the `while-guard-limit` key in `~/.specs`.

### `--EXP-UTF8`

**Experimental** — enable UTF-8 aware character counting. When this flag is set, multi-byte UTF-8 sequences are treated as a single logical character for the purposes of character-range selection, word start/end positions, and output column calculations. This flag is marked experimental because it is not fully tested across all features.

### `--debug-alu-comp`

*(Debug builds only.)* Print detailed information about the parsing and compilation of ALU expressions. Useful for diagnosing why an expression is being parsed differently than expected.

### `--debug-alu-run`

*(Debug builds only.)* Print step-by-step evaluation information for ALU expressions at runtime. Very verbose; useful for tracing a complex expression's intermediate values.

### `--timezone name`

Convert to and from time-formatted strings using the named timezone. Values come from the TZ database, such as `America/New_York`, `Europe/London`, or `Asia/Tokyo`. See [Wikipedia](https://en.wikipedia.org/wiki/List_of_tz_database_time_zones) for the full list. This can also be set in the configuration file.

### `--regexType optionList`

Set the regular expression grammar. The default is `ECMAScript`. Other options include `basic`, `extended`, `awk`, `grep`, and `egrep`. You can also combine flags like `icase` for case-insensitive matching.

## Python Functions

### `--pythonFuncs on/off/auto`

Control loading of Python functions (see Chapter 14). The default, `auto`, loads Python only when an unknown function name is encountered. Set to `off` to disable Python entirely (also needed if Python support was not compiled in). Set to `on` to always load Python even when no unknown functions appear.

### `--pythonErr throw/NaN/zero/nullstr`

Determine what happens when a Python function raises an exception. The default, `throw`, causes specs to abort with an error message. Alternatives: `NaN`, `zero`, and `nullstr` return a safe value instead of aborting.

## Help and Information

### `--help topic`

Print help for a topic without running specs. Topics include: `help` (how to use this switch), `pyfuncs` (Python functions), `builtin` (built-in functions), `specs` (saved specifications), or the name of a specific function or saved specification.

### `--info`

Print build information: version, platform, Python version, compiler, build source, commit hash, and build time.

```
specs --info
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

# Chapter 4: The Configuration File

The **configuration file** is a plain-text file that defines *configured literals* — named constants that specs can use in specifications. It also contains settings that affect specs's default behavior.

## Location

- **Linux and macOS**: `~/.specs` (in your home directory)
- **Windows**: `%HOME%\specs.cfg`

You can override this with the `-c` flag (Chapter 3).

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

## Settings that Belong in the Configuration File

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

### `SPECSPATH`

A colon-separated list of directories where specs looks for spec files (Chapter 5) and Python function files (Chapter 14):
```
SPECSPATH: /home/alice/specs:/usr/local/share/specs
```
Defaults to `$HOME/specs` if not set.

### `pythonDisabled`

Set to `1` to permanently disable Python function loading. Unlike `--pythonFuncs off`, this cannot be overridden from the command line:
```
pythonDisabled: 1
```

### `NO_WARN_REDEFINED_FID`

Set to any value to suppress the warning that specs emits when a field identifier is re-defined within the same spec:
```
NO_WARN_REDEFINED_FID: 1
```

### `EmptyFrequencyMapMessage`

The string returned by `fmap_dump()` when the frequency map contains no data:
```
EmptyFrequencyMapMessage: (no data)
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
| `@!` | The context-affected input record (see Chapter 13) |
| `@+n` / `@-n` | Record at offset +n / -n from current (see Chapter 13) |

## Ensuring a Literal is Defined — REQUIRES

If your specification depends on a configured literal, you can protect it with `REQUIRES`:

```
REQUIRES pi
specs r: word 1 .
     PRINT "@pi*r*r" 1
```

If `pi` is not defined in `~/.specs`, specs will abort with a clear error message rather than silently producing wrong results.

---

# Chapter 5: Offline Specifications — Spec Files

When a specification grows beyond a few data fields, putting it all on the command line becomes unwieldy. **Spec files** let you write specifications in a file with comments, indentation, and blank lines.

## Using a Spec File

```
specs -f myspec.txt < input.txt
```

or equivalently:

```
specs --specFile myspec.txt < input.txt
```

The `-f` flag can appear anywhere before the spec units (but since spec files replace spec units, there are typically no spec units on the command line when `-f` is used).

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

### Comments

There are two styles of comments in spec files:

1. **Full-line comment**: The line begins with `# ` (hash, space). The entire line is ignored.
2. **End-of-line comment**: The comment begins at the last occurrence of ` # ` (space, hash, space) that is also preceded by whitespace. Everything after it is ignored.

```
w1 1           # This puts the first word at column 1
/hello/ nextword   # This appends the word "hello"
```

Note: `# ` must be preceded by whitespace for an end-of-line comment. A hash inside a literal string (`/hello # world/`) is not a comment.

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

## The SPECSPATH

When you specify a relative filename with `-f`, specs searches for it in the **SPECSPATH** — a colon-separated list of directories. The SPECSPATH is controlled by:

1. The `SPECSPATH` environment variable
2. The `SPECSPATH` entry in `~/.specs`
3. The default: `$HOME/specs` on Linux/macOS, `%APPDATA%\specs` on Windows

So if your `SPECSPATH` is `/home/alice/specs`, you can store your spec files there and reference them as `-f myscript` without specifying a full path.

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

## Saving Specifications for Later Use

You can document your spec files with docstrings and list them with `--help specs`:

```
specs --help specs
```

This lists all spec files found on the `SPECSPATH`. To get help on a specific one:

```
specs --help myspec
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

# Chapter 6: Selecting and Transforming Input

Every data field begins with an **input source** — a description of where to get the data for this field. This chapter covers all input source types and the conversions that can be applied to them.

## Character Ranges

The simplest input source is a range of character positions (1-based):

| Syntax | Meaning |
|--------|---------|
| `n` | Single character at position n |
| `n-m` | Characters n through m inclusive |
| `n.len` | `len` characters starting at position n |
| `n;m` or `n:m` | Same as `n-m` (alternative separators) |
| `-n` | n-th character from the end (`-1` is the last) |
| `1-*` | The entire record |

The semicolon (`;`) separator is included for CMS Pipelines compatibility. The colon (`:`) is particularly useful when the ending position is negative, making it more readable (`1:-3` means from position 1 to 3 before the end).

## Words

Words are separated by the **word separator**, which defaults to any locale-defined whitespace character.

| Syntax | Meaning |
|--------|---------|
| `w1` or `word 1` | First word |
| `w-1` or `word -1` | Last word |
| `w2-4` or `words 2-4` | Words 2 through 4 |
| `w2.3` | 3 words starting at word 2 |

The entire input, from the start of the first word to the end of the last specified word (including any separators between them), is captured as the value. So `w1-3` on `"  hello   world   foo  "` gives `hello   world   foo`.

## Fields

Fields are separated by the **field separator**, which defaults to a tab character. Unlike words, the separator between fields is exactly one character, so empty fields are possible.

| Syntax | Meaning |
|--------|---------|
| `f1` or `field 1` | First field |
| `f-1` or `field -1` | Last field |
| `f2-4` or `fields 2-4` | Fields 2 through 4 |
| `f2.3` | 3 fields starting at field 2 |

On a tab-separated input `a\t\tb`, field 1 is `a`, field 2 is empty, and field 3 is `b`.

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

Both `WORDSEPARATOR` and `FIELDSEPARATOR` are **MainOptions** — they apply to the entire specification and must appear before any data fields.

## The SUBSTRING Spec Unit

For more complex selection, use `SUBSTRING` (or `SUBSTR`):

```
SUBSTRING [WORDSEP char] [FIELDSEP char] range OF InputSource
```

This selects a range (character, word, or field) *within* another input source. The `WORDSEP` and `FIELDSEP` options let you use different separators just for this substring selection, without changing the global setting.

**Example**: Extract the filename from `ls -l` output. The full path (`/Applications/Safari.app/Contents/Resources/en.lproj/foo.html`) is the last word. We want the last component (after the last slash):

```
ls -l | specs substr fieldsep / field -1 of word -1    1
```

For the record `-rw-r--r--  1 root  wheel  2554 Oct 30 09:46 /path/to/foo.html`, this outputs `foo.html`.

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

A floating-point number of seconds since the Unix epoch, representing the time the current specs run started:

```
echo "" | specs todclock 1
```
Output: `1736000000.000000` (approximately, depending on when you run it)

### `DTODclock`

Like `TODclock`, but gives the time of producing the **current** output record rather than the start of the run.

### `TIMEDIFF`

A 12-character decimal number giving microseconds since the start of the run. Useful for timing:

```
echo -e "first\nsecond\nthird" | specs timediff 1
```

### The `PRINT` / `?` Source

Evaluates an ALU expression and uses the result as input. Covered in detail in Chapter 8.

### `ID fieldIdentifier`

Uses the stored value of a **field identifier** as the input source. Field identifiers are covered next.

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
echo "" | specs x48454C4C4F 1
```
Output: `HELLO`

Hex literals can be combined with conversions like `C2X` to round-trip binary data, or used to inject separator characters that are hard to type on the command line.

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
echo "Oct 30 09:46" | specs w1-3 tf2s "%b %d %H:%M" 1
```
Output: `1572421560.000000`

### STRIP

Adding `STRIP` between the input source and the conversion (or output placement) removes leading and trailing whitespace from the value before placing it:

```
echo "  hello  " | specs 1-* strip 1
```
Output: `hello`

## Record Formats — When Records Are Not Lines

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
specs w1 1 --recfm D --linedel '|' < data.pipe-separated
```

### `--recfm F` — Fixed Length

```
specs ... --recfm F --lrecl N
```

specs reads exactly **N** bytes from the input stream per record, regardless of what those bytes are. There are no separators; the record boundary is purely positional. This is the right choice for tightly packed binary data, certain instrument outputs, or mainframe flat files where each record occupies a known, constant number of bytes.

`--lrecl` (logical record length) is **required** with `--recfm F`.

```
# Each record is exactly 80 bytes — no newlines needed
specs 1-10 1 21-30 nw --recfm F --lrecl 80 < punched-card-image.bin
```

Note that character positions are still 1-based from the start of each record, so column 1 is the first byte of each 80-byte chunk, column 80 is the last, and so on.

### `--recfm FD` — Fixed Length with Delimiter

```
specs ... --recfm FD --lrecl N
```

A hybrid: specs reads one delimited line at a time (using the OS line-ending or a custom `--linedel`), but then treats the result as a fixed-length field of exactly **N** characters. Lines longer than N are truncated; lines shorter than N are padded with spaces on the right. This is useful for data that has line endings for human readability but whose fields are always at fixed positions within those lines.

```
# Lines may vary in actual length but are logically 132 characters wide
specs 1-10 1 101-110 nw --recfm FD --lrecl 132 < report.txt
```

### Summary Table

| `--recfm` | Nickname | `--lrecl` | `--linedel` | What a "record" is |
|-----------|----------|-----------|-------------|---------------------|
| `D` | delimited | not used | optional | One line (custom or OS delimiter) |
| `F` | fixed | required | not used | Exactly `--lrecl` bytes, no delimiter |
| `FD` | fixed-delimited | required | optional | One delimited line, normalized to `--lrecl` chars |

---

# Chapter 7: Placing Output

The **output placement** tells specs where to put the result of an input source in the output record.

## Absolute Placement

Specify a column number:

```
echo "hello world" | specs w1 1 w2 10
```
Output: `hello    world`

The output record is padded with spaces to reach column 10.

You can also specify a range to constrain the width:

```
echo "hello world" | specs w1 1-5
```
Output: `hello` (5 characters at column 1)

If the value is shorter than the range, it is padded (left-aligned by default). If longer, it is truncated.

## Relative Placement

| Keyword | Meaning |
|---------|---------|
| `n` or `next` | Immediately after the previous output, no gap |
| `nw` or `nextword` | After one space following the previous output |
| `nf` or `nextfield` | After one tab following the previous output |
| `.` | No output — the value is captured but not placed (used with field identifiers) |

The abbreviated forms (`n`, `nw`, `nf`) also accept alternate spellings `nword` and `nfield`.

### Width Suffix on Relative Placement

Any of `next`, `nextword`, and `nextfield` can take an optional width suffix in the form `.N`, making the output a fixed-width column at the relative position:

| Syntax | Meaning |
|--------|---------|
| `nw.10` or `nextword.10` | Next-word position, 10-character column |
| `nf.8` or `nextfield.8` | Next-field position, 8-character column |
| `n.5` or `next.5` | Next position, 5-character column |

This is equivalent to using absolute `nextword` placement followed by an explicit width range but expressed more compactly. Alignment applies after the column width (default left-aligned; add `right` or `center` to change it).

```
echo "hello world" | specs w1 nw.10 right  w2 nw.10 right
```
Output: `     hello     world`

`next` places output exactly where the previous output ended:

```
echo "AB" | specs /[/ 1 1-* n /]/ n
```
Output: `[AB]`

`nextword` inserts a space:

```
echo "hello world" | specs w1 1 w2 nw
```
Output: `hello world`

## Alignment

After the output placement, you can specify an alignment for values shorter than the output field:

| Keyword | Effect |
|---------|--------|
| `left` | Pad on the right (default) |
| `right` | Pad on the left |
| `center` or `centre` | Pad equally on both sides |

```
echo "hello" | specs 1-* 1.20 center
```
Output: `       hello        `

```
echo "42" | specs 1-* 1-10 right
```
Output: `        42`

## The PAD Spec Unit — Changing the Padding Character

By default, gaps in the output record (spaces between placed fields, or padding added to make a value fit a fixed-width column) are filled with the **space character**. The `PAD` spec unit changes this fill character for all subsequent output in the same specification.

```
PAD /char/
```

The argument is a single character, using any delimiter. You can also write it without a delimiter if the character is unambiguous (e.g., `PAD *`).

`PAD` can appear multiple times to use different padding for different fields.

**Example — three different padding characters:**

```
echo "The quick brown" | specs pad /q/ w1 1.10 left  pad /w/ w2 11.10 center  pad /e/ w3 21.10 right
```
Output: `Theqqqqqqqwwquickwwweeeeebrown`

Breaking this down:
- `pad /q/` sets padding to `q`; word 1 (`The`) placed left-aligned in a 10-char field → `Theqqqqqqq`
- `pad /w/` sets padding to `w`; word 2 (`quick`) placed center-aligned in 10 chars → `wwquickwww`
- `pad /e/` sets padding to `e`; word 3 (`brown`) placed right-aligned in 10 chars → `eeeeebrown`

**Example — fill the gap between two fields:**

```
echo "First record" | specs word 1 5  pad *  word 2 15
```
Output: `    First*****record`

Here, the space from column 10 (end of "First") to column 14 (before "record") is filled with `*` instead of spaces.

`PAD` is a **MainOption** — it takes effect immediately where it appears and applies to all subsequent output until another `PAD` changes it again.

## Composed Output Placement

A **composed output placement** uses a parenthesized expression to compute the starting column, width, and alignment dynamically at runtime. It replaces both the regular output placement and alignment arguments:

```
InputSource (start_expr)
InputSource (start_expr, width_expr)
InputSource (start_expr, width_expr, align_expr)
```

The expressions are ALU expressions (Chapter 8) evaluated each cycle.

Examples:

```
# Right-align a word in a 20-character field
specs w1 (20-len(word(1)))
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

### Eliding Arguments

You can omit leading arguments and use commas as placeholders. Omitting the start defaults to `next()`, and omitting the width defaults to `rest()`:

```
specs w1 (,,'R')   # next, full width, right-aligned
```

---

# Chapter 8: Expressions and the ALU

The **Arithmetic-Logical Unit (ALU)** is specs's computation engine. It evaluates *expressions* — combinations of values, operators, and function calls — to produce a result.

## Where Expressions Appear

Expressions appear in:

- `PRINT "expression"` — compute a value and place it in the output
- `SET "#n op expression"` — compute a value and store it in a counter
- `IF "condition"`, `WHILE "condition"` — control flow (Chapter 10)
- Composed output placement arguments (Chapter 7)

## The PRINT Spec Unit

`PRINT` is the expression-to-output bridge. It evaluates its argument as an ALU expression and places the result in the output.

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

When a field identifier (say `a`) is set, you can use it by name in expressions:

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

Named persistent variables also exist: see `pset()` and `pget()` in Chapter 9.

### Configured Literals

`@name` refers to a value from the configuration file:

```
# In ~/.specs:  pi: 3.14159265
specs PRINT "@pi * 2" 1
```
Output: `6.28318530`

Note: `@pi` used *outside* of an expression (like `specs @pi 1`) is expanded to the raw string `3.14159265` before specs parses it, so it becomes a string literal, not a numeric expression. Inside an expression (inside quotes after `PRINT`, `SET`, `IF`, etc.), it is a numeric value.

### The Entire Record — `@@` and `@!`

- `@@` — the entire current input record (always the original, unaffected by `CONTEXT`)
- `@!` — the current input record as affected by `CONTEXT` (same as `@@` when no `CONTEXT` is active)

```
echo "hello world" | specs PRINT "len(@@)" 1
```
Output: `11`

### Record Offsets — `@+n` and `@-n`

In expressions, `@+n` refers to the record n positions ahead and `@-n` to n positions behind (see Chapter 13 on rolling context).

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
| `\|\|` | Concatenation | `'hello' \|\| ' ' \|\| 'world'` → `hello world` |

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
| `\|` | Logical OR | `1` if either operand is non-zero |

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
| `\|\|=` | Append (string concatenation) |

```
echo -e "1\n2\n3" | specs SET "#0:=0" EOF PRINT "#0" 1
```
This is wrong — the `SET` runs for every record, resetting #0 each time. To initialize a counter, use `IF "first()"`:

```
echo -e "1\n2\n3" | specs a: w1 . SET "#0+=a" EOF PRINT "#0" 1
```
Output: `6` (counters start at zero automatically)

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

---

# Chapter 9: Built-in Functions

specs provides a large library of built-in functions for use in expressions. This chapter presents them organized by category.

## Calling Functions

Functions appear inside ALU expressions:

```
specs PRINT "len('hello')" 1
```
Output: `5`

Multiple arguments are separated by commas:
```
specs PRINT "substr('hello world', 7, 5)" 1
```
Output: `world`

## Numerical and Logical Functions

| Function | Description |
|----------|-------------|
| `abs(x)` | Absolute value |
| `ceil(x)` | Smallest integer ≥ x |
| `floor(x)` | Largest integer ≤ x |
| `round(x, d)` | Round x to d decimal places (default: nearest integer) |
| `sign(x)` | 1 if x > 0, 0 if x = 0, -1 if x < 0 |
| `sqrt(x)` | Square root (returns NaN for negative x) |
| `exp(x)` | e^x |
| `log(x, base)` | Logarithm (default: natural log) |
| `pow(x, y)` | x raised to the power y |
| `sin(x)` | Sine (x in radians) |
| `cos(x)` | Cosine (x in radians) |
| `tan(x)` | Tangent (x in radians) |
| `arcsin(x)` | Inverse sine (result in radians) |
| `arccos(x)` | Inverse cosine (result in radians) |
| `arctan(x)` | Inverse tangent (result in radians) |
| `dsin(x)` | Sine (x in degrees) |
| `dcos(x)` | Cosine (x in degrees) |
| `dtan(x)` | Tangent (x in degrees) |
| `arcdsin(x)` | Inverse sine (result in degrees) |
| `arcdcos(x)` | Inverse cosine (result in degrees) |
| `arcdtan(x)` | Inverse tangent (result in degrees) |
| `fact(n)` | n factorial |
| `combinations(n, k)` | Number of ways to choose k from n |
| `permutations(n, k)` | Number of ordered ways to choose k from n |
| `rand(x)` | Random integer 0 ≤ result < x. If x omitted: random float [0, 1) |
| `not(expr)` | 1 if expr is zero; 0 otherwise |
| `c2d(x)` | Signed decimal value of binary string x (up to 8 bytes, 2's complement) |
| `c2u(x)` | Unsigned decimal value of binary string x (up to 8 bytes) |
| `c2f(x)` | Floating-point value of binary string x |
| `frombin(x)` | Decimal value of binary string x (little-endian) |
| `tobin(x)` | Binary (byte) representation of integer x |
| `tobine(x, n)` | Binary representation of integer x as n-byte string |
| `fmt(value, format, digits, decimal, separator)` | Format a number. `format`: `f` for fixed, `s` for scientific |
| `pretty(value, flimit, ilimit, locale)` | Format with thousands separators |

## String Functions

| Function | Description |
|----------|-------------|
| `len(s)` or `length(s)` | Length of string |
| `left(s, n)` | Leftmost n characters (space-padded if shorter) |
| `right(s, n)` | Rightmost n characters (space-padded if shorter) |
| `center(s, n)` / `centre(s, n)` | Center n characters (space-padded if shorter) |
| `substr(s, start, len)` | Substring of s starting at start for len characters |
| `pos(needle, haystack)` | 1-based position of first occurrence of needle in haystack; haystack defaults to current record |
| `lastpos(needle, haystack)` | 1-based position of last occurrence of needle; haystack defaults to current record |
| `includes(haystack, needle1, ...)` | 1 if any needle is a substring of haystack; haystack defaults to current record |
| `includesall(haystack, needle1, ...)` | 1 if all needles are substrings of haystack |
| `substitute(haystack, needle, subst, max)` | Replace needle with subst, up to max times (`U` = all) |
| `reverse(s)` | Reverse the characters in s |
| `strip(s, option, pad)` | Strip leading/trailing characters. option: `B` (both, default), `L` (leading), `T` (trailing) |
| `space(s, len, pad)` | Replace internal whitespace with len occurrences of pad |
| `copies(s, n)` | s repeated n times |
| `translate(s, tableout, tablein, pad)` | Translate characters in s according to mapping |
| `abbrev(h, n, l)` | 1 if first l chars of n equal first chars of h |
| `compare(s1, s2, pad)` | Index of first mismatch, or 0 if equal |
| `verify(s, ref, option, start)` | Find first character of s not in ref (N) or in ref (M) |
| `insert(s, target, pos, len, pad)` | Insert s into target at position pos |
| `overlay(s1, s2, start, len, pad)` | Overlay s1 onto s2 starting at start |
| `delstr(s, start, len)` | Delete len characters from s starting at start |
| `justify(s, len, pad)` | Evenly justify words within s to length len |
| `lvalue(s, sep)` | Left-hand side of `key=value` string (sep defaults to `=`) |
| `rvalue(s, sep)` | Right-hand side of `key=value` string (sep defaults to `=`) |
| `xrange(start, end)` | String of all characters from start to end |
| `x2d(s, len)` | Hex string to decimal (signed if len missing or ≤ 0) |
| `rmatch(s, regex, flags)` | 1 if regex matches all of s |
| `rsearch(s, regex, flags)` | 1 if regex matches any substring of s |
| `rreplace(s, regex, fmt, flags)` | Replace all regex matches in s with fmt |

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

## REXX-Derived String Functions

| Function | Description |
|----------|-------------|
| `bitand(x, y)` | Bitwise AND of strings x and y |
| `bitor(x, y)` | Bitwise OR of strings x and y |
| `bitxor(x, y)` | Bitwise XOR of strings x and y |
| `find(string, phrase)` | Word number of first occurrence of phrase in string |
| `index(haystack, needle, start)` | Character position of needle in haystack |
| `wordpos(phrase, string, start)` | Word number where phrase begins in string |
| `words(string)` | Number of blank-delimited words in string |
| `subword(string, start, len)` | Substring starting at word start for len words |
| `wordindex(string, n)` | Character position of word n |
| `wordlength(string, n)` | Length of word n |
| `delword(string, start, len)` | Delete len words starting at word start |
| `sword(str, n, sep)` | n-th word using first char of sep as separator |
| `sfield(str, n, sep)` | n-th field using first char of sep as separator |
| `wordwith(substr)` | First word of current record containing substr |
| `wordwithidx(substr)` | Index of first word containing substr |
| `fieldwith(substr)` | First field containing substr |
| `fieldwithidx(substr)` | Index of first field containing substr |

## Record Access Functions

These functions read from the current (or context-affected) input record:

| Function | Description |
|----------|-------------|
| `record()` | Entire current input record (context-affected; same as `@!`) |
| `cfrecord()` | Entire current input record, ignoring context (same as `@@`) |
| `range(n, m)` | Characters n through m of the current record |
| `word(n)` | n-th word of the current record |
| `wordrange(n, m)` | Words n through m |
| `wordcount(s, p)` | Number of words in s (default: current record) |
| `wordstart(n)` | Start position of word n |
| `wordend(n)` | End position of word n |
| `wordlen(n)` | Length of word n |
| `field(n)` | n-th field of the current record |
| `fieldrange(n, m)` | Fields n through m |
| `fieldcount(s, p)` | Number of fields in s (default: current record) |
| `fieldindex(n)` | Start position of field n |
| `fieldend(n)` | End position of field n |
| `fieldlength(n)` | Length of field n |
| `recno()` | Current record number (number of records read so far) |
| `number()` | Number of processing cycles completed |
| `ctxrecno()` | Record number of the context-affected record |
| `ctxoffset()` | Current context offset (0 if no CONTEXT is active) |
| `ctxoob(s)` | 1 if s came from out-of-bounds input |
| `split(sep, hdr, ftr)` | All fields on separate output lines |
| `splitw(sep, hdr, ftr)` | All words on separate output lines |
| `splus(s, o, l)` | Substring at offset o from first instance of string s |
| `wplus(s, o, l)` | Substring at word offset o from first occurrence of word s |
| `fplus(s, o, l)` | Substring at field offset o from first occurrence of field s |
| `next()` | Current print position (where next output would go) |
| `rest()` | Number of columns from the current print position to the terminal width (`@cols − next() + 1`) |

## Time Functions

| Function | Description |
|----------|-------------|
| `tf2mcs(s, f)` | Parse time string s with format f → microseconds since epoch |
| `mcs2tf(x, f)` | Format microseconds-since-epoch x with format f → string |
| `tf2s(s, f)` | Parse time string s with format f → seconds since epoch |
| `s2tf(x, f)` | Format seconds-since-epoch x with format f → string |

Time format strings follow strftime conventions. The additional format code `%xf` (where x is 0–6) produces fractional seconds with x digits.

Example:
```
echo "2024-01-15 14:32:07" | specs PRINT "tf2mcs(word(1)||' '||word(2),'%Y-%m-%d %H:%M:%S')" 1
```
Output: `1705329127000000`

## Statistical and Frequency Map Functions

These functions work with **field identifiers** to accumulate statistics across records:

| Function | Description |
|----------|-------------|
| `present(a)` | 1 if field identifier a is assigned, 0 otherwise |
| `sum(a)` | Sum of all values assigned to field identifier a |
| `min(a)` | Minimum value assigned to a |
| `max(a)` | Maximum value assigned to a |
| `average(a)` | Arithmetic mean of values assigned to a |
| `variance(a)` | Variance of values assigned to a |
| `stddev(a)` | Standard deviation of values assigned to a |
| `stderrmean(a)` | Standard error of the mean of values assigned to a |
| `fmap_nelem(a)` | Number of distinct values of a |
| `fmap_nsamples(a)` | Total number of samples of a |
| `fmap_common(a)` | Most frequent value of a |
| `fmap_rare(a)` | Least frequent (but non-zero) value of a |
| `fmap_count(a, s)` | Number of times value s appeared for a |
| `fmap_frac(a, s)` | Fraction of samples of a that equal s |
| `fmap_pct(a, s)` | Percentage of samples of a that equal s |
| `fmap_sample(a, s)` | Record s as a new sample for a; returns count |
| `fmap_dump(a, format, sortOrder, showPct)` | Formatted dump of frequency map |
| `countocc(needle, haystack)` | Times needle appeared in haystack across all calls |
| `countocc_get(needle)` | Count of needle without a new match attempt |
| `countocc_dump(format, sortOrder, showPct)` | Dump of countocc frequency map |

### fmap_dump Parameters

- **format**: `txt`/`0`/empty (default), `lin` (boxed), an integer (fixed width), `csv`, `json`
- **sortOrder**: `s`/`sa` (alphabetical asc, default), `sd` (alphabetical desc), `c`/`ca` (count asc), `cd` (count desc)
- **showPct**: boolean — if true, adds a percentage column

Example — word frequency counter:

```
cat file.txt | specs a: w1 . EOF PRINT "fmap_dump(a,'csv','cd',1)" 1
```

## Shell Command Functions

These functions run a shell command and let you process its standard output, standard error, and return code:

| Function | Description |
|----------|-------------|
| `exec(cmd)` | Run shell command c; returns its standard output (one trailing newline stripped) |
| `exc1(cmd, [lineNo])` | Like `exec`, but returns only the content of output line `lineNo`; `lineNo` must be a positive integer and defaults to 1; empty string if absent |
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
specs PRINT "exec('grep XXDATE *')" WRITE "stderr" WRITE PRINT "excerr()"
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

## Special Functions

| Function | Description |
|----------|-------------|
| `first()` | 1 during the first iteration, 0 otherwise |
| `eof()` | 1 during the run-out (post-input) cycle, 0 otherwise |
| `conf(key, default)` | Value of configured literal key; default if missing; NaN if no default given |
| `defined(key)` | 1 if configured literal key is defined, 0 otherwise |
| `getenv(name)` | Value of environment variable name; NaN if undefined |
| `pset(var, value)` | Set persistent variable var to value; returns value |
| `pget(var, default)` | Get persistent variable var; returns default if not set |
| `pdefined(var)` | 1 if persistent variable var is defined |
| `pclear(var)` | Clear persistent variable var; returns its former value |
| `string(x)` | Force x to be stored as a string |
| `exact(expr)` | 1 if expr evaluated without rounding, 0 otherwise |
| `break(a)` | 1 if break level a is established this cycle (see Chapter 11) |

### Persistent Variables

`pset` and `pget` store values that persist across separate invocations of specs. They are stored on disk (in the user's home directory or a similar location) and survive between sessions.

```
# Count how many times you've run a script
specs PRINT "pset('runs', pget('runs',0)+1)" 1
```

An abbreviated form: `#varname` is equivalent to `pget('varname')`.

---

# Chapter 10: Control Flow

By default, specs executes all spec units in the specification sequentially for every input record. Control flow lets you make decisions and repeat operations.

## Conditions

A **condition** is an expression that evaluates to true or false:
- True: any non-zero, non-NaN, non-empty value
- False: zero, NaN, or empty string

Any ALU expression can be a condition.

## IF / THEN / ELSE / ENDIF

```
IF "condition" THEN
    unit-sequence
ELSEIF "condition" THEN
    unit-sequence
ELSE
    unit-sequence
ENDIF
```

Rules:
- `THEN` is required after every condition.
- `ELSEIF` blocks are optional; you can have as many as you like.
- `ELSE` is optional.
- `ENDIF` is required. Omitting it may sometimes work, but nested conditionals become ambiguous.

**Simple example**: Print whether each number is odd or even:

```
echo -e "1\n2\n3\n4" | specs a: w1 1 /is/ nextword IF "a%2" THEN /odd/ nextword ELSE /even/ nextword ENDIF
```
Output:
```
1 is odd
2 is even
3 is odd
4 is even
```

**Multi-line in a spec file**:

```
a: w1 1
   /is/ nextword
   IF "a%2" THEN
       /odd/ nextword
   ELSE
       /even/ nextword
   ENDIF
```

## WHILE / DO / DONE

```
WHILE "condition" DO
    unit-sequence
DONE
```

As long as the condition is true, the unit sequence is repeated. **Warning**: specs has a **while-guard** that aborts after a large number of iterations to prevent accidental infinite loops. Disable it with `--no-while-guard` if you intentionally need a long loop.

**Example**: Print n asterisks for each input number:

```
echo -e "1\n3\n5" | specs a: w1 1 SET "#1:=a" WHILE "#1>0" DO /\*/ n SET "#1-=1" DONE
```
Output:
```
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
specs IF "word(1)=='#'" THEN CONTINUE ENDIF 1-* 1
```

## ABEND — ABnormal END

`ABEND` halts the entire specs run with an error message sent to stderr. The current output record is *not* written. No further records are processed.

```
specs
   a: 55-57 NW
   IF "a > 120" THEN
       ABEND /Age in record exceeds limit/
   ENDIF
```

`ABEND` only makes sense inside a conditional.

## ASSERT

`ASSERT` is like `ABEND` but takes a condition rather than a message. If the condition is false, specs aborts with a message that includes the text of the failed condition:

```
specs
   a: 55-57 NW
   ASSERT "a <= 120"
```

If `a` is 150, specs outputs: `Assertion failed: a <= 120` and aborts.

Use `ASSERT` for sanity checks that should always be true. Use `ABEND` for conditions you want to handle with a specific error message.

## SKIP-WHILE and SKIP-UNTIL

These cause specs to skip input records while a condition holds (or until it holds). Once the condition for passing is satisfied once, future records are never re-evaluated — all subsequent records pass through.

```
SKIP-WHILE "condition"    # skip while condition is true
SKIP-UNTIL "condition"    # skip until condition becomes true
```

They work like `CONTINUE` within an implicit `IF`, but they "turn off" after the first record passes.

**Example**: Print all records from the first one whose date is after July 1st, 2020:

```
a: w1 .        # date in yyyymmdd format
SKIP-WHILE "a < 20200701"
1-* 1
```

These spec units usually make sense at the beginning of a specification.

---

# Chapter 11: Run-In, Run-Out, and Control Breaks

## The Normal Cycle

Each iteration of specs processes one input record. The spec units run, output is produced, and the next record is read. This continues until input is exhausted.

## Run-In: The First Iteration

The first iteration is called the **run-in** cycle. You can detect it with the `first()` function:

```
IF "first()" THEN
    # initialization
ENDIF
```

Useful for setting up counters or recording the first record's values:

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

### The `eof()` Function — Forced Run-Out

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

### When to Use `eof()` vs. `EOF`

Both accomplish the same thing. Style preference:

- Use `EOF` when the run-out logic is clearly separated from the per-record logic — it reads cleanly in a spec file.
- Use `eof()` inside `IF` when the logic is interleaved with per-record processing.

## PRINTONLY and KEEP

`PRINTONLY` (a **MainOption**) suppresses per-record output unless a specified break level is established. It is followed by either:

- A **field identifier** (case matters) — records are suppressed until that break level is established (see Control Breaks below)
- The keyword **`EOF`** — records are suppressed until the input is exhausted (i.e., only the run-out output is printed)

```
PRINTONLY EOF
# ... per-record spec units (all output suppressed) ...
EOF
   # ... summary output (printed) ...
```

`KEEP` (always following `PRINTONLY`) prevents the output buffer from being cleared between non-printed records. This allows output from multiple records to accumulate into a single output record before the break level is established.

## Control Breaks

A **control break** occurs when a tracked field identifier changes value from one record to the next. This is a pattern common in report generation — printing a header whenever a category changes.

### Setting Up a Break

Use a field identifier with a `BREAK` statement:

```
specs
    FIELDSEPARATOR ,
    c: FIELD 1   .
       FIELD 3  10
       /,/      NEXT
       FIELD 2  NEXTWORD
    BREAK c
        ID c 1
```

Here, `c` is set to the first field (department name) of each CSV record. When `c` changes, the break is triggered, and `ID c 1` prints the department name. When `c` stays the same, the department name is suppressed.

Result from a personnel CSV (department, first, last):
```
Payroll  Eck, Eddy
         Polen, Janel
Finance  Cockett, Leonard
         Lugo, Dorie
...
```

### How Break Levels Work

When the value of a field identifier changes, the **break level** is set to that identifier. The break level and all identifiers alphabetically lower than it are said to be **established**.

For example: if field identifiers `a`, `b`, and `c` are tracked, and `b` changes, then break levels `a` and `b` are established, but `c` is not.

Spec units following a `BREAK x` statement execute only when break level x or higher is established.

### The `break()` Function

The `break(a)` function returns 1 if break level `a` is established, and 0 otherwise. It can be used in `IF` conditions for more complex break handling:

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
...
```

---

# Chapter 12: Multiple Records and Streams

The default one-in-one-out model can be broken in many ways. This chapter covers them all.

## Multiple Output Records — WRITE

Use the `WRITE` spec unit to emit the current output record and start a fresh one, all within the same cycle:

```
cat ls-output.txt | specs
    /Filename:/ 1
    w-1         nw
    WRITE
    /type:/ 1
    IF "range(1,1)=='d'" THEN
        /directory/ nw
    ELSE
        /file/ nw
    ENDIF
```

`WRITE` flushes the current output record and clears the output buffer. The last record in a cycle is always written automatically — you don't need a trailing `WRITE`.

## Suppressing Output — NOWRITE and NOPRINT

`NOWRITE` (synonymous with `NOPRINT`) suppresses the automatic output record at the end of the cycle. No output is written for this record unless you used an explicit `WRITE` earlier.

```
# Only write records that contain "error"
specs
    IF "includes('error')" THEN
        1-* 1
    ELSE
        NOWRITE
    ENDIF
```

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
specs
    IF "first()" THEN
        SET "#4:=word(2)"    # save the first commit hash
    ELSE
        PRINT "#4" 1         # print the saved hash
        WORD 2        NEXTWORD
        READ
        WORD 2-6 tf2s "%c" NEXTWORD
        WHILE "word(1)!='commit'" DO
            READSTOP
        DONE
        SET "#4:=word(2)"    # save the next commit hash
    ENDIF
```

## UNREAD — Pushing Back a Record

`UNREAD` pushes the current record back so it will be re-read as the first record of the next cycle. This avoids the "consumed one record too many" problem when looping with `READ`/`READSTOP`:

```
specs
    WORD 2                    1         # author username
    READSTOP                            # read past commit line
    WORD 2             NEXTWORD         # email
    READSTOP
    WORD 2-6 tf2s "%c" NEXTWORD        # date
    WHILE "word(1)!='commit'" DO
        READSTOP
    DONE
    UNREAD                              # push back the next "commit" line
```

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

## Multiple Input Streams

Assign additional input files with `--is2` through `--is8`. At each cycle, one record is read from each stream simultaneously.

```
specs -i file1.txt --is2 file2.txt WORD 1 1 WORD 2 NW SELECT 2 WORD 2 NW
```

This joins the second column of two matching files into a three-column output.

Switch between streams with `SELECT n` (where n is the stream number 1–8). The stream resets to #1 at the start of each new cycle.

**When all streams have equal record counts**, this is the simplest way to join data. If stream lengths differ, the `STOP` MainOption controls behavior:

- `STOP ALLEOF` (default): continue until all streams are exhausted; shorter streams emit empty records
- `STOP ANYEOF`: stop when any stream runs out
- `STOP n`: stop when stream n runs out

## Multiple Output Streams

Assign additional output files with `--os2` through `--os8`. Switch between them with `OUTSTREAM n` or `OUTSTREAM STDERR`:

```
specs -o main.txt --os2 errors.txt
    IF "includes('ERROR')" THEN
        OUTSTREAM 2
        1-* 1
        OUTSTREAM 1
    ELSE
        1-* 1
    ENDIF
```

---

# Chapter 13: Rolling Context

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

On input `alpha`, `beta`, `gamma`:
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

When the context record doesn't exist (before the first record or past the last), it is empty.

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

Note: Reading out-of-bounds with `@+n` or `@-n` does not stop processing, even if `READSTOP` is present.

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

On input `alpha`, `beta`, `gamma`:
```
beta
alpha
gamma
beta

gamma
```

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

# Chapter 14: Python Functions

## When to Write a Python Function

specs has a rich library of built-in functions, but sometimes you need functionality that isn't there. Python functions let you add arbitrary functions by writing them in Python.

Write a Python function when:
- You need an algorithm that's easier to express in Python (complex string processing, external libraries, stateful computations)
- You want to use Python's standard library (`re`, `math`, `datetime`, etc.)
- You need to maintain state across records that's more complex than a counter

## The localfuncs.py File

All Python functions must be defined in a file called **`localfuncs.py`**. This file must reside in a directory on the **SPECSPATH** (see Chapter 4 and 5). It is a regular Python file that you can `import` from the Python environment.

**All functions whose names do not begin with an underscore (`_`)** are automatically available to specs. Use underscore-prefixed names for helper functions that you don't want to expose.

## A First Example

Suppose you want a `commas` function that formats integers with thousands separators:

```python
def commas(x):
    '''Convert the integer x into a string with thousands groups separated by commas'''
    x = int(x)
    ret = ""
    while x >= 1000:
        rm = str(x % 1000)
        x = x // 1000
        while len(rm) < 3:
            rm = "0" + rm
        ret = "," + rm + ret
    if x > 0:
        ret = str(x) + ret
    return ret
```

Place this in `$HOME/specs/localfuncs.py`. Now use it in specs:

```
echo "1398234" | specs print "commas(word(1))" 1.12 right
```
Output: `   1,398,234`

## Importing Python Modules

Your functions can use any module available in the Python environment:

```python
import math
import datetime

def prime_factors(n):
    '''Return the prime factors of n as a comma-separated string'''
    n = int(n)
    factors = []
    d = 2
    while d * d <= n:
        while n % d == 0:
            factors.append(str(d))
            n //= d
        d += 1
    if n > 1:
        factors.append(str(n))
    return ','.join(factors)
```

## Stateful Functions

Python functions can maintain state using module-level variables:

```python
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

```python
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

```python
def commas(x):
    '''Convert the integer x into a string with thousands groups separated by commas'''
    ...
```

Then:
```
specs --help pyfuncs
```
Output:
```
Python Interface Functions:
===========================
- commas (x) :  Convert the integer x into a string with thousands groups separated by commas
```

## Python Function Options

### --pythonFuncs on/off/auto

Controls when Python functions are loaded:

- `auto` (default): Load Python only if specs encounters an unknown function name. This avoids the overhead of initializing Python for specs that don't need it.
- `on`: Always load Python, even if no Python functions are called.
- `off`: Never load Python. Calling a Python function is an error.

### --pythonErr throw/NaN/zero/nullstr

Controls what happens when a Python function raises an exception:

- `throw` (default): specs aborts with an error message echoing the Python traceback.
- `NaN`: Return NaN and continue.
- `zero`: Return the integer 0 and continue.
- `nullstr`: Return an empty string and continue.

Use `NaN` or `nullstr` when you want specs to tolerate Python errors gracefully.

## Exactness

specs tracks whether numerical values are **exact** (no rounding has occurred) or **inexact**. This is used by functions like `exact()` to let you reason about precision.

### Return Exactness

By default:
- Integer return values are **exact**
- String return values are **exact**
- Float return values are **inexact**

To override, return a 2-tuple `(value, bool)` instead of a plain value:

```python
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

```python
def lowindex(x):
    '''Returns the lower 16 bits, preserving exactness'''
    return (int(x[0]) % 65536, x[1])

lowindex.arg_type = "exact"
```

When `arg_type = "exact"` is set, **all** arguments are passed as 2-tuples `(value, is_exact)`.

### Combining Argument and Return Exactness

```python
def add_exact(a, b):
    '''Add two numbers, exact only if both inputs are exact'''
    return (a[0] + b[0], a[1] and b[1])

add_exact.arg_type = "exact"
```

### Error Handling

If `arg_type` is set to an unrecognized value, specs reports an error at initialization:

```python
def bad_function(x):
    pass

bad_function.arg_type = "bogus"  # ERROR at startup
```

If a function with `arg_type = "exact"` tries to use an argument as a plain value (not as a tuple), Python raises a `TypeError`, which specs reports.

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
cat filelist.txt | specs /gzip -9/ 1 w1 nw --shell
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

Use `--threaded` when:
- Processing large files (millions of records)
- I/O is the bottleneck (slow disks, network filesystems)
- CPU-intensive specifications (complex ALU expressions)

Do NOT use `--threaded` with:
- Rolling context (`CONTEXT`)
- Multiple input streams

---

# Appendix B: Quick Reference

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
| `--threaded` | `-t` | Run in threaded mode |
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

## Input Record Formats (--recfm)

| Format | Nickname | lrecl | linedel | Notes |
|--------|----------|-------|---------|-------|
| `D` | delimited | n/a | optional | Default. Records end at OS line separator |
| `F` | fixed | required | n/a | Exactly lrecl characters per record |
| `FD` | fixed-delimited | required | optional | Fixed length with line delimiter |

## Input Source Syntax

| Syntax | Example | Meaning |
|--------|---------|---------|
| `n` | `5` | Character at position 5 |
| `n-m` | `3-7` | Characters 3 to 7 |
| `n.len` | `5.8` | 8 characters starting at 5 |
| `-n` | `-1` | Last character |
| `1-*` | `1-*` | Entire record |
| `wn` / `word n` | `w2` | n-th word |
| `wn-m` / `words n-m` | `w1-3` | Words n through m |
| `w-1` | `w-1` | Last word |
| `fn` / `field n` | `f3` | n-th field |
| `fn-m` / `fields n-m` | `f2-4` | Fields n through m |
| `SUBSTR ... OF ...` | | Substring of another source |
| `PRINT "expr"` / `? "expr"` | | ALU expression result |
| `ID fid` | `ID a` | Value of field identifier |
| `/text/` | `/hello/` | String literal |
| `xHH...` | `x4142` | Hex literal: binary string from hex pairs |
| `RECNO` / `NUMBER` | | Record counter (10 digits) |
| `TODclock` | | Seconds since epoch (start of run) |
| `DTODclock` | | Seconds since epoch (current record) |
| `TIMEDIFF` | | Microseconds since start of run |

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

## Alignment

| Keyword | Effect |
|---------|--------|
| `left` | Left-align (default) |
| `right` | Right-align |
| `center` / `centre` | Center |

## Conversions

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
| `WORDSEPARATOR str` | Set word separator characters |
| `FIELDSEPARATOR str` | Set field separator characters |
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
| `\|\|` | Concatenate | |
| `<` `<=` `>` `>=` | Numeric compare | |
| `=` `!=` | Smart equality | Numeric if both numeric |
| `==` `!==` | Strict equality | Always string |
| `<<` `<<=` `>>` `>>=` | String compare | Alphabetical |
| `!` | Logical NOT | |
| `&` | Logical AND | |
| `\|` | Logical OR | |

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
| `\|\|=` | Append string |

---

*This guidebook covers specs version XXVERSION. The specs project is hosted at [https://github.com/yoavnir/specs2016](https://github.com/yoavnir/specs2016).*
