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
           WORD 2-6 tf2d "%c" NEXTWORD
           WHILE "word(1)!='commit'" DO
              READ
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
