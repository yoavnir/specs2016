# Command-Line Switches

Here's a list of the command-line switches available for *specs*:
* `--toASCII` -- causes output to be translated into ASCII if it's outside the range.
Any character that is not ASCII-printable is rendered as a period.
* `--force-read-input` -- forces **specs** to read records even if none of the spec units reference input records.  By default they won't.
Not reading a line is useful for simple specifications such as: `specs @version 1` or `specs print "2+2" 1`.
* `--specFile` **filename** or `-f` **filename** -- reads the specification from a file rather than the command line.
Using files allows you to write some very sophisticated specifications, and to aid readability by using indentation. For example, consider the following specification from the CMS Pipelines book.  That would be unwieldy to write on a single command-line:
```
# This counts and adds up a list of numbers
    printonly eof
    a: word 1
       set "#0+=a" # Counter #0 is an accumulator
       set "#1+=1" # Counter #1 is a counter
    eof
       /Total:/   1
       print #0 strip nextword
       /in/           nextword
       print #1 strip nextword
       /records./     nextword
``` 
Using files also allows you to include comments. The rules for comments are that either the line begins with a hash mark and a space, and then the entire line is a comment; or the last occurrence of a hash mark and a space is also preceded by a space, and that is where the comment starts. 
You can specify the full path of the files, or `specs` will search the **SPECSPATH** for them. The **SPECSPATH** can be set from either the environment variable `SPECSPATH` or the configuration string `SPECSPATH`. In both cases the syntax is just like the OS `PATH`: a list of directories separated by OS-specific path separator character. On Linux and Mac OS this is a colon (`:`). On Windows this is a semicolon (`;`).
* `--verbose` or `-v` -- outputs more information when something goes wrong.
* `--stats` -- output statistics on run time, records read, and records written to standard output. 
The resulting stats look something like this:
```
Read  608913 lines.
Wrote 608913 lines.
Run Time: 11.5247 seconds.
CPU Time: 20.1237872 seconds.
Main Thread:
	Initializing: 3.144 ms (0.027%)
	Processing: 10.000 seconds (86.767%)
	Waiting on input queue: 203.551 ms (1.766%)
	Waiting on output queue: 1.318 seconds (11.439%)
	Draining: 111.065 us (0.001%)
Reader Thread:
	Initializing: 84.135 us (0.001%)
	Processing: 334.285 ms (2.901%)
	Waiting on IO: 1.549 seconds (13.442%)
	Waiting on output queue: 9.634 seconds (83.612%)
	Draining: 5.036 ms (0.044%)
Writer Thread:
	Initializing: 2.514 ms (0.022%)
	Processing: 748.592 ms (6.496%)
	Waiting on IO: 4.794 seconds (41.602%)
	Waiting on input queue: 5.979 seconds (51.880%)
	Draining: 48.686 us (0.000%)
```
* `--inFile` **filename** or `-i` **filename** -- get the input records from a file rather than standard input.
* `--outFile` **filename** or `-o` **filename** -- write the output records to a file rather than standard output.
* `--spaceWS` -- Makes **specs** only treat spaces as the default word separator. By default all locale-defined whitespace is treated as the word separator.
* `--debug-alu-comp` -- Prints out detailed information about the parsing and compiling of expressions (_only in debug build_).
* `--debug-alu-run` -- Prints out detailed step-by-step information about the evaluation of expressions (_only in debug build_).
* `--timezone` **name** -- convert to and from time-formatted strings using the selected timezone. Valid values are from the TZ database and look like `Africa/Dakar`, `America/Chicago`, `Asia/Calcutta`, `Australia/Sydney`, or `Europe/Berlin`.  A full list of such timezones is available on [Wikipedia](https://en.wikipedia.org/wiki/List_of_tz_database_time_zones).  Note that the same timezones can also be configured in the config
* `--config` **filename** or `-c` **filename** -- overrides the default configuration file which is `~/.specs` on POSIX-based operating systems (Mac OS and Linux) or `%HOME%\specs.cfg` on Windows.
* `--set` **name=value** or `-s` **name=value** -- sets the named string *name* to the value *value*.
* `--is2` **filename** -- sets input stream number 2 to read from the specified file. 
* `--is3` **filename** -- sets input stream number 3 to read from the specified file. 
* `--is4` **filename** -- sets input stream number 4 to read from the specified file. 
* `--is5` **filename** -- sets input stream number 5 to read from the specified file. 
* `--is6` **filename** -- sets input stream number 6 to read from the specified file. 
* `--is7` **filename** -- sets input stream number 7 to read from the specified file. 
* `--is8` **filename** -- sets input stream number 8 to read from the specified file. 
* `--os2` **filename** -- sets output stream number 2 to write to the specified file.
* `--os3` **filename** -- sets output stream number 3 to write to the specified file.
* `--os4` **filename** -- sets output stream number 4 to write to the specified file.
* `--os5` **filename** -- sets output stream number 5 to write to the specified file.
* `--os6` **filename** -- sets output stream number 6 to write to the specified file.
* `--os7` **filename** -- sets output stream number 7 to write to the specified file.
* `--os8` **filename** -- sets output stream number 8 to write to the specified file.
* `--recfm` **format** -- sets the format of the primary input stream. See below table for supported formats.
* `--lrecl` **record-length** -- sets the length of each record. Relevant to *fixed* and *fixed-delimited* records.
* `--linedel` **delimiter** -- sets the line delimiter for input records on the primary stream.
* `--regexType` **syntaxOptionList** -- Sets the syntax option for regular expressions. The parameter is a comma-separated list of syntax options. See the table below for a list of valid syntax options.
* `--pythonFuncs` **on/off/auto** -- Enables of disables the loading of Python functions. **auto**, which is the default signifies that Python functions are loaded only when the parser encounters an unknown function. Note that setting the `pythonDisabled` configured literal to `1` will disable Python functions and cannot be overridden from the command line.
* `--pythonErr` **throw/NaN/zero/nullstr** -- determines what happens when a called Python function throws an exception. The default, `throw` is for **specs** to throw its own exception and terminate. The alternatives, `NaN`, `zero`, and `nullstr` make **specs** behave as if the function returned, NaN, the integer zero, or an empty string respectively.

## Table of record formats

|recfm|nickname|lrecl|linedel|comment|
|-----|--------|-----|-------|-------|
| `D` | delimited | *n/a* | *optional* | This is the default. Records are delimited by the OS line separator if *linedel* is not specified. |
| `F` | fixed | *required* | *n/a* | **specs** will read exactly *lrecl* characters from the input stream regardless of what those characters may be. |
| `FD` | fixed-delimited | *required* | *optional* | **specs** will read one line at a time delimited by the OS line separator if *linedel* is not specified. Lines that are longer than *lrecl* will be truncated; lines that are shorter will be padded by spaces. |

## Table of syntax options

|option|effects on syntax|Notes|
|------|-----------------|-----|
| `icase` | Case Insensitive | Regular expression matches do not regard case |
| `nosubs` | No sub-expressions | Not relevant here -- included for completeness |
| `optimize` | Optimize matching | Matching efficiency is preferred over construction efficiency |
| `collate` | Locale sensitiveness | Character ranges like `[a-b]` are affected by locale |
| `ECMAScript` | ECMAScript grammar | The regular expression follows this grammar. Do not specify more than one of these |
| `basic` | Basic POSIX grammar | The regular expression follows this grammar. Do not specify more than one of these |
| `extended` | Extended POSIX grammar | The regular expression follows this grammar. Do not specify more than one of these |
| `awk` | Awk POSIX grammar | The regular expression follows this grammar. Do not specify more than one of these |
| `grep` | Grep POSIX grammar | The regular expression follows this grammar. Do not specify more than one of these |
| `egrep` | Egrep POSIX grammar | The regular expression follows this grammar. Do not specify more than one of these |

