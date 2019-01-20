# Command-Line Switches

Here's a list of the command-line switches available for *specs*:
* `--toASCII` -- causes output to be translated into ASCII if it's outside the range.
Any character that is not ASCII-printable is rendered as a period.
* `--force-read-input` -- forces specs to read every input line even if none of the spec units use it.  By default they won't.
Not reading a line is useful for simple specifications such as:
    `specs @version 1`
    `specs print "2+2" 1`
* `--specFile` **filename** or `-f` **filename** -- reads the specification from a file rather than the command line.
Using files allows you to write some very sophisticated specifications, and to aid readability by using indentation. For example, consider the following specification from the CMS Pipelines book.  That would be unwieldy to write on a single command-line:
```
specs 
    printonly eof
    a: word 1
       set (#0+=a; #1+=1)
    eof
       /Total:/   1
       print #0 strip nextword
       /in/           nextword
       print #1 strip nextword
       /records./     nextword
``` 
**Version 0.2 Note:** The above is not yet supported, as neither `printonly eof` not multiple assignments in a single **set** are supported in this version.
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
* `--spaceWS` -- Makes the program treat the only spaces as the default word separator. Otherwise all locale-defined whitespace is treated as the default word separator.
* `--debug-alu-comp` -- Prints out detailed information about the parsing and compiling of expressions (_only in debug build_).
* `--debug-alu-run` -- Prints out detailed step-by-step information about the evaluation of expressions (_only in debug build_).
