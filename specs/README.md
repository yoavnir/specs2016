# specs2016
A re-writing of the specs pipeline stage from CMS, only changed quite a bit

"specs" is a command line utility for parsing and re-arranging text
input. It allows re-alignment of fields, some format conversion, and
re-formatting multiple lines into single lines or vice versa. Input
comes from standard input, and output flows to standard output.

This version is based on the **CMS Pipelines User's Guide and Reference** ([link](https://publib.boulder.ibm.com/epubs/pdf/hcsj0c30.pdf)), especially chapters 16, 24, and 20.

The command line format is as follows:

  specs [switches] [spec-units]
  
Switches
========
(To Be Added) 
   
Spec Units
==========
Each spec unit specifies an action to be taken by the program. The spec unit may span from one to four command line arguments.

The ordinary spec unit is a *data field*, which consists of four arguments, two of which may be omitted:

    InputPart [conversion] OutputPart [alignment]
  
The **InputPart** argument may be any of the following:

* A range of characters, such as `5`, `3-7`, or `5.8`, the last one indicating 8 characters starting in the 5th position. Note that the indexing of characters is 1- rather than 0-based.
* A range of words, such as `w5` or `words 5-7`, where words are separated by one or more `wordseparator` characters -- spaces by default. The word indexing is 1-based.
* A range of fields, such as `fields 5` or `f5-7`, where fields are separated by exactly one `fieldseparator` characters -- a tab by default. The field indexing is 1-based.
* **TODclock** - a 64-bit formatted timestamp, giving nanoseconds since the Unix epoch.
* **DTODclock** - a 64-bit formatted timestamp, giving nanoseconds since the Unix epoch. The difference is that TODclock shows the time when this run of *specs* begun, while DTODclock gives the time of producing the current record.
* **NUMBER** - A record counter as a 10-digit decimal number.
* **TIMEDIFF** - an 8-char decimal number indicating the number of seconds since the invocation of the program.
* a string literal, optionally enclosed by delimiters, such as `/TODclock/` or `'NUMBER'`. Note that to include the single quotes on the command line requires you to enclose them in double quotes.
    
The **OutputPart** argument specifies where to put the source:

* absolute position (such as `1`)
* range (such as `1-5` or `1.5`)
* `n` or `next` for placing the output immediately after the 
    previous output.
* `nw` or `nextword` for placing the output following a space 
    character after the previous output.
* `nf` or `nextfield` for placing the output following a tab 
    character after the previous output.
    
The alignment argument can be `l`, `c`, or `r`, for "left", "center", and "right" respectively.

The conversion argument can specify any of the following conversions:

* **rot13** - encrypts the bytes using the ROT-13 cipher
* **x2d** - converts hex data to decimal
* **d2x** - converts decimal data to hex
* **b2x** - converts binary data to hex
* **x2b** - converts hex data to binary
* **x2bt** - converts hex data to binary, padded with zeros to include all nibbles.
* **ucase** - converts text to uppercase
* **lcase** - converts text to lowercase
 
There are also four special spec units, that may be used:

* **read** - causes the program to read the next line of input. If we have already read the last line, the read line is taken to be the empty string.
* **readstop** - causes the program to read the next line of input. If we have already read the last line, no more processing is done for this iteration.
* **write**- causes the program to write to output and reset the output 
      line.
* **redo** - causes the program to use the line of output as the new input, and reset the output line.
      
Example:

   `ls -l` yields this:

    total 352
    -rw-r--r--@ 1 ynir  admin    574 Aug 25  2009 Makefile
    -rw-r--r--@ 1 ynir  admin   3542 Nov 23 00:21 README
    -rw-r--r--@ 1 ynir  admin    362 Nov 19 08:31 conversion.h
    -rw-r--r--  1 ynir  admin    984 Nov 11 17:45 ls.txt
    -rw-r--r--@ 1 ynir  admin   2233 Nov 23 00:03 main.cc
    -rw-r--r--  1 ynir  admin   9412 Nov 23 00:11 main.o
    -rw-r--r--@ 1 ynir  admin   6567 Nov 23 00:09 spec_build.cc
    -rw-r--r--  1 ynir  admin  16776 Nov 23 00:11 spec_build.o
    -rw-r--r--@ 1 ynir  admin   5494 Nov 19 08:30 spec_convert.cc
    -rw-r--r--  1 ynir  admin  17004 Nov 23 00:11 spec_convert.o
    -rw-r--r--@ 1 ynir  admin  11419 Nov 23 00:10 spec_params.cc
    -rw-r--r--  1 ynir  admin  21080 Nov 23 00:11 spec_params.o
    -rw-r--r--@ 1 ynir  admin    375 Nov 11 09:29 spec_vars.cc
    -rw-r--r--  1 ynir  admin   4800 Nov 23 00:11 spec_vars.o
    -rwxr-xr-x  1 ynir  admin  36740 Nov 23 00:11 specs
    -rw-r--r--@ 1 ynir  admin   1547 Nov 23 00:10 specs.h

Let's run it though a spec:

    ls -l | specs 12-* 1 redo w2 1 w4 d2x 8.8 r w8 17
The first spec unit converts it to this:

    1 ynir  admin    574 Aug 25  2009 Makefile
    1 ynir  admin   3542 Nov 23 00:21 README
    1 ynir  admin    362 Nov 19 08:31 conversion.h
    1 ynir  admin    984 Nov 11 17:45 ls.txt
    1 ynir  admin   2233 Nov 23 00:03 main.cc
    1 ynir  admin   9412 Nov 23 00:11 main.o
    1 ynir  admin   6567 Nov 23 00:09 spec_build.cc
    1 ynir  admin  16776 Nov 23 00:11 spec_build.o
    1 ynir  admin   5494 Nov 19 08:30 spec_convert.cc
    1 ynir  admin  17004 Nov 23 00:11 spec_convert.o
    1 ynir  admin  11419 Nov 23 00:10 spec_params.cc
    1 ynir  admin  21080 Nov 23 00:11 spec_params.o
    1 ynir  admin    375 Nov 11 09:29 spec_vars.cc
    1 ynir  admin   4800 Nov 23 00:11 spec_vars.o
    1 ynir  admin  36740 Nov 23 00:11 specs
    1 ynir  admin   1547 Nov 23 00:10 specs.h

Then after the redo, we get this:

    ynir        23e Makefile
    ynir        dd6 README
    ynir        16a conversion.h
    ynir        3d8 ls.txt
    ynir        8b9 main.cc
    ynir       24c4 main.o
    ynir       19a7 spec_build.cc
    ynir       4188 spec_build.o
    ynir       1576 spec_convert.cc
    ynir       426c spec_convert.o
    ynir       2c9b spec_params.cc
    ynir       5258 spec_params.o
    ynir        eae spec_vars.cc
    ynir       12c0 spec_vars.o
    ynir       8f84 specs
    ynir        60b specs.h

      
Alternatively, let's arrange this on multiple lines:

    ls -l | specs w9 1 write "Owner:" 3 w3 10 write "Size:" 3 w5 10-20 r

    Makefile
      Owner: ynir
      Size:          574
    README
      Owner: ynir
      Size:         5834
    conversion.h
      Owner: ynir
      Size:          362
    list.txt
      Owner: ynir
      Size:          978
    ls.txt
      Owner: ynir
      Size:          984
    main.cc
      Owner: ynir
      Size:         2233
    main.o
      Owner: ynir
      Size:         9412

Finally, let's make our own version of the multi-column display:

    ls -l | specs w9 1 read w9 26 read w9 51
                             Makefile                 README
    conversion.h             main.cc                  main.o
    spec_build.cc            spec_build.o             spec_convert.cc
    spec_convert.o           spec_params.cc           spec_params.o
    spec_vars.cc             spec_vars.o              specs
    specs.h

Differences from CMS Pipelines **specs**
===
* A colon (`:`) can be used in a from-to range, in addition to the hyphen and semicolon supported in CMS Pipelines, as in `5:7` or `5:-5`.
* **TimeDiff** does not exist in CMS pipelines.
* **ReDo** does not exist in CMS pipelines.
* **TODclock** - The mainframe version uses the ESA/390 architecture TOD clock, where the high-order 32 bits represent the time since the 1/1/1900 epoch in units of 1.048576 seconds and the other 32-bit represent fractions of this unit. We use the Unis epoch (based at 1/1/1970) with a 64-bit field of nanoseconds.
* **DTODclock** does not exist in CMS pipelines.
* No support for multiple input and output streams.
* Support tail labels in addition to the head labels from CMS Pipelines

