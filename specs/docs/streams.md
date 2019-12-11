# Streams and Records
Normally, each iteration of the specification consumes one input record from standard input, and produces one output record to standard output. There are, however, exceptions.

## >1 Output Record in Each Iteration
Sometimes we would like to produce more than one output record. We use the `WRITE` keyword for that. Here's a contrived example:
```
specs    /Filename:/ 1 
         word -1     nw
         WRITE
         /type:/ 1
         if "range(1,1)=='d'" then
            /directory/ nw
         else
            /file/ nw
```
**Note:** There is no need for a WRITE for the last output record in the specification, because the last record is always output.

| Input | Output |
| ----- | ------ |
| -rw-r--r--   1 synp  staff  1404 Jan 25 00:14 Makefile | Filename: Makefile |
| | type: file |
| drwxr-xr-x   9 synp  staff   288 Jan 25 10:44 cli | Filename: cli |
| | type: directory |

Every WRITE resets the output record to empty and it can get filled again from scratch.

## No Output Records At All
Sometimes we would like to not produce any output record at all. For that we use the `NOWRITE` keyword or its synonym `NOPRINT`. So why do we have data fields at all if we don't want to output them?  There can be several reasons:
1. We may be writing a specification whose only output is in the run-out phase, but we have some output for each record as debugging output. When the specification works as we want it to, we add the `NOWRITE` token, eliminating all per-record output.
1. We may want to write the record or not write it based on some condition. The `NOWRITE` token can appear within an `IF` statement.

## >1 Input Records in Each Iteration
Sometimes we would like to use more than one input record to produce our output record. We use the `READ` or `READSTOP` keywords for that.

Both `READ` and `READSTOP` read the next record from the input stream to be the new active input record. The difference is what to do if the current line was the last. With `READ` the specification continues to be executed as if we have just read an empty record. With `READSTOP` the execution of the specification stops.

Below is an example of a specification that handles git log. A git log looks something like this:
```
commit df3438ed9e95c2aa37a429ab07f0956164ec4229
Author: synp71 <yoav.nir@gmail.com>
Date:   Sun Jan 20 21:40:41 2019 +0200

    Add NEWS section to Readme.md

commit e6d7f9ac591379d653a5685f9d75deccc1792545
Author: synp71 <yoav.nir@gmail.com>
Date:   Sun Jan 20 21:09:47 2019 +0200

    Issue #33: Some more docs improvement
    
    Also fixed the stats to conform to current timestamp format.

commit 241002cf5a66737bbfd29888244a0a463cd9bcae
Author: synp71 <yoav.nir@gmail.com>
Date:   Thu Jan 17 23:45:21 2019 +0200

    Issue #33: fix formatting

commit 9efb13277c561a3a28195d469420031add60946e
Author: synp71 <yoav.nir@gmail.com>
Date:   Thu Jan 17 23:38:01 2019 +0200

    Issue #33 basic specification and CLI switches
```

The goal is for each commit to print the commit hash, the author username, and the date and time in internal format. The challenging part is that there is a variable number of lines between a "Date:" record and the next "commit" record.  We can READ with a loop, but by the time we know we're done, we've already consumed the next commit record. We'll get around this difficulty by using the variable `#4` to hold the commit hash.

```
specs   IF "first()" THEN
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

The result is:
```
df3438ed9e95c2aa37a429ab07f0956164ec4229 synp71 1548013241.000000
e6d7f9ac591379d653a5685f9d75deccc1792545 synp71 1548011387.000000
241002cf5a66737bbfd29888244a0a463cd9bcae synp71 1547761521.000000
```

## Pushing Back The Last Record
That specification in the previous section reads several lines in a `WHILE` loop searching for the line we need for the next iteration. This is a common pattern and we were forced to use a variable to transfer the content of the next commit record to the next iteration.

**specs** version 0.3 introduces the `UNREAD` spec unit. What it does is push back the current read record so that it is possible to process it as the first record of the next iteration. The specification above can thus be simplified as follows:

```
specs WORD 2                    1
      READSTOP
      WORD 2             NEXTWORD
      READSTOP
      WORD 2-6 tf2s "%c" NEXTWORD
      WHILE "word(1)!='commit'" DO
          READSTOP
      DONE
      UNREAD
```

## Process a Record in Two Phases
Sometimes it's easier to pre-process a record with one specification, and then take the result and process it with another specification. For example, support you are processing the result of `grep` on a bunch of files. You get records that look like this:
```
test8:mmind.wariat.org - - [04/Jul/1995:08:12:26 -0400] "GET /shuttle/countdown/video/livevideo.gif HTTP/1.0" 304 0
test8:bruosh01.brussels.hp.com - - [04/Jul/1995:08:12:26 -0400] "GET /shuttle/missions/sts-71/mission-sts-71.html HTTP/1.0" 200 12418
test8:beastie-ppp1.knoware.nl - - [04/Jul/1995:08:12:26 -0400] "GET /shuttle/missions/sts-71/images/KSC-95EC-0423.txt HTTP/1.0" 200 1224
test8:piweba3y.prodigy.com - - [04/Jul/1995:08:12:28 -0400] "GET /shuttle/countdown/liftoff.html HTTP/1.0" 200 4535
test8:sullivan.connix.com - - [04/Jul/1995:08:12:28 -0400] "GET /shuttle/missions/sts-71/images/index71.gif HTTP/1.0" 200 57344
test8:bruosh01.brussels.hp.com - - [04/Jul/1995:08:12:33 -0400] "GET /shuttle/missions/sts-71/sts-71-patch-small.gif HTTP/1.0" 200 12054
test9:mmind.wariat.org - - [04/Jul/1995:08:12:33 -0400] "GET /shuttle/countdown/liftoff.html HTTP/1.0" 304 0
test9:www-d4.proxy.aol.com - - [04/Jul/1995:08:12:34 -0400] "GET /shuttle/missions/sts-71/sts-71-day-01-highlights.html HTTP/1.0" 200 2722
test9:mmind.wariat.org - - [04/Jul/1995:08:12:35 -0400] "GET /shuttle/countdown/video/livevideo.gif HTTP/1.0" 304 0
test9:eepc50.ee.surrey.ac.uk - - [04/Jul/1995:08:12:35 -0400] "GET /shuttle/countdown/video/livevideo.jpeg HTTP/1.0" 200 50437
test10:piweba3y.prodigy.com - - [04/Jul/1995:08:12:37 -0400] "GET /shuttle/countdown/video/livevideo.gif HTTP/1.0" 200 61490
test10:crocus-fddi.csv.warwick.ac.uk - - [04/Jul/1995:08:12:39 -0400] "GET /shuttle/missions/sts-71/mission-sts-71.html HTTP/1.0" 200 12418
test10:crocus-fddi.csv.warwick.ac.uk - - [04/Jul/1995:08:12:41 -0400] "GET /shuttle/missions/sts-71/sts-71-patch-small.gif HTTP/1.0" 200 12054
```
We want to process the first word of every file, but the filename at the start gets in the way. It is separated by a colon, but there are other colons at the continuation of the line. The simplest way is to use to separate specifications like this:
```
grep shuttle test* | specs fs : f2-* 1 | specs /source:/ 1 w1 nw
```
But this seems inelegant. **specs** includes the `REDO` spec unit just for this. It takes the current output line, and converts it to be the current input line. The rest of the specification creates a new output line based on that. Here is what our example looks like with `REDO`:
```
grep shuttle test* | specs fs : f2-* 1 REDO /source:/ 1 w1 nw
```

## The Second Reading Station
At the conclusion of each cycle, **specs** loads the record from the primary input into a buffer, called the *second reading station*, that can be accessed during the next cycle.  Similar to the `EOF` token and the `eof()` function, any access to the second reading forces a *run-out cycle*.

The second reading is accessed using the keywords `SELECT SECOND`.  You can return to reading the primary input stream using `SELECT FIRST`.

Consider the following input:
```
first record
second line
last one
```
And use the following specification:
```
specs WORD 1        1
      SELECT SECOND
      WORD 1 NEXTWORD
      SELECT FIRST
      WORD 2 NEXTWORD
      SELECT SECOND
      WORD 2 NEXTWORD
```
The output is:
```
first record
second first line record
last second one line
last one
```
A few things to note:
1. The fourth line comes from the run-out cycle that has nothing in the primary input but has the last input line in the secondary reading.
1. `NEXTWORD` with an empty argument does not leave a space. This is why the last output record has only one space between the words.
1. `NEXTWORD` begins at column 1 if the output record is empty. That is why the last output record does not begin with a space.
1. It does not matter what the selected stream is at the end of the specification. The next cycle always begins with the primary stream selected.
1. `READ` and `READSTOP` **MUST NOT** be used during secondary reading. This will result in an error.
1. Specifications should not mix `READ` and `READSTOP` with `SELECT SECOND` even if the `READ` or `READSTOP` is during reading of the primary record. The results are undefined and may change in future releases.

## Multiple Input Streams
**specs** allows you to use multiple input streams in your specifications. The way this works is that you use the `--is2` to `--is8` CLI switches to specify additional (up to a total of 8) input streams to use. At each cycle of the specification, 1 record is read from each input stream, which implies that the number of records in each stream should be equal. 

The multiple input streams is mostly useful in collating data, because the input stream records need to be matched.

The way you use multiple streams is by using the `SELECT` keyword followed by a stream number. For example, suppose we have two input files as follows:

| file1 | file2 |
| ----- | ----- |
| Alice  164 | Alice 65 |
| Bob    178 | Bob   82 |
| Carol  171 | Carol 66 |
| Eve    169 | Eve   68 |

Both files have two words, but we'd like to combine them into three-word records. Here's how to do it:

`specs -i file1 --is2 file2 WORD 1 1  WORD 2 NW  SELECT 2  WORD 2 NW`

At the start of a new cycle, the active stream resets to #1. The end result will look like this:
```
Alice 164 65
Bob 178 82
Carol 171 66
Eve 169 68
```

## Multiple Output Streams
**specs** allows you to use multiple output streams as well. Similar to input streams, you can use the `--os2` to `--os8` CLI switches to assign file names to the numbered output streams.  There is an additional defined output stream called `STDERR` which outputs to the stderr stream of the **specs** invocation.

The way you select among the multiple output streams is by using the `OUTSTREAM` keyword followed by either a stream number or the `STDERR` keyword.











