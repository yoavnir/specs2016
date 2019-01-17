# Command-Line Switches

Here's a list of the command-line switches available for *specs*:
* `--toASCII` -- causes output to be translated into ASCII if it's outside the range.
* `--force-read-input` -- forces specs to read every input line even if none of the spec units use it.  By default they won't.
* `--specFile **filename**` or `-f **filename**` -- reads the specification from a file rather than the command line.
* `--verbose` or `-v` -- outputs more information when something goes wrong.
* `--stats` -- output statistics on run time, and records read, and on records written. 
* `--inFile **filename**` or `-i **filename**` -- get the input records from a file rather than standard input.
* `--outFile **filename**` or `-o **filename**` -- write the output records to a file rather than standard output.
* `--spaceWS` -- Makes the program treat the only spaces as the default word separator. Otherwise all locale-defined whitespace is treated as the default word separator.
* `--debug-alu-comp` -- Prints out detailed information about the parsing and compiling of expressions (_only in debug build_).
* `--debug-alu-run` -- Prints out detailed step-by-step information about the evaluation of expressions (_only in debug build_).
