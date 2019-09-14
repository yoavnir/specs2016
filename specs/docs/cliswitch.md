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
       set (#0+=a; #1+=1) # Counter #0 is an accumulator; #1 is a counter
    eof
       /Total:/   1
       print #0 strip nextword
       /in/           nextword
       print #1 strip nextword
       /records./     nextword
``` 
**Version 0.5 Note:** The above is not yet supported, as neither `printonly eof` not multiple assignments in a single **set** are supported in this version.
Using files also allows you to include comments. The rules for comments are that either the line begins with a hash mark and a space, and then the entire line is a comment; or the last occurrence of a hash mark and a space is also preceded by a space, and that is where the comment starts. 
You can specify the full path of the files, or `specs` will search the **SPECSPATH** for them. The **SPECSPATH** can be set from either the environment variable `SPECSPATH` or the configuration string `SPECSPATH`. In both cases the syntax is just like the OS `PATH`: a list of directories separated by colons.
* `--verbose` or `-v` -- outputs more information when something goes wrong.
* `--stats` -- output statistics on run time, records read, and records written to standard output. 
The resulting stats look something like this:
```
Read  1891715 lines.
Wrote 1891715 lines.
Run Time: 46.811 seconds.
CPU Time: 69.1277526 seconds.
```
* `--inFile` **filename** or `-i` **filename** -- get the input records from a file rather than standard input.
* `--outFile` **filename** or `-o` **filename** -- write the output records to a file rather than standard output.
* `--spaceWS` -- Makes **specs** only treat spaces as the default word separator. By default all locale-defined whitespace is treated as the word separator.
* `--debug-alu-comp` -- Prints out detailed information about the parsing and compiling of expressions (_only in debug build_).
* `--debug-alu-run` -- Prints out detailed step-by-step information about the evaluation of expressions (_only in debug build_).
* `--timezone` **name** -- convert to and from time-formatted strings using the selected timezone. Valid values are from the TZ database and look like `Africa/Dakar`, `America/Chicago`, `Asia/Calcutta`, `Australia/Sydney`, or `Europe/Berlin`.  A full list of such timezones is available on [Wikipedia](https://en.wikipedia.org/wiki/List_of_tz_database_time_zones).  Note that the same timezones can also be configured in the config
* `--config` **filename** or `-c` **filename** -- overrides the default configuration file which is `~/.specs` on POSIX-based operating systems (Mac OS and Linux) or `%HOME%\specs.cfg` on Windows.
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

## Table of record formats

|recfm|nickname|lrecl|linedel|comment|
|-----|--------|-----|-------|-------|
| `D` | delimited | *n/a* | *optional* | This is the default. Records are delimited by the OS line separator if *linedel* is not specified. |
| `F` | fixed | *required* | *n/a* | **specs** will read exactly *lrecl* characters from the input stream regardless of what those characters may be. |
| `FD` | fixed-delimited | *required* | *optional* | **specs** will read one line at a time delimited by the OS line separator if *linedel* is not specified. Lines that are longer than *lrecl* will be truncated; lines that are shorter will be padded by spaces. |



