# Sample specification, used to eyeball specs.xml syntax highlighting.
# It is not meant to do anything useful, but every construct here is valid.
+SET today date "+%Y-%m-%d"

           FIELDSEPARATOR ,
           WORDSEPARATOR default
           PAD "0"
           CONTEXT 1

        a: WORD 1            1-8
        b: w2-4              10.20      LEFT
           f-1               NEXTWORD   RIGHT
           SUBSTRING 3-7 OF FIELD 2     nw.8
           1-*               .
           x48656c6c6f        NEXT
           /1f/              x2d        NEXTFIELD
           w6-8              tf2s "%b %d %H:%M"    NEXTWORD
           @today            (40,20,'C')
           /literal text/    NEXTWORD   CENTER
           ID a              nw

           SET "#0 += b; #1 := #0 * 100"
           PRINT "substitute(@@,XXVERSION,@version,'U')"   NEXTWORD
           PRINT "@+1 || '/' || @!"   NEXTWORD
           ?length(record())   nextword
           IF "wordcount(@@) > 3 & !first()" THEN
              PRINT "fmt(sum(a) / recno(),'f',8,'.',',')"   nextword
           ELSEIF "eof()" THEN
              /run-out phase/   NEXTWORD
           ELSE
              ASSERT "recno() > 0"
              IF "recno() < 0" THEN
                 ABEND "'this can never happen'"
              ENDIF
           ENDIF

           WHILE "pget('more',0)" DO
              READSTOP
              SET "#2 := #2 - 1"
           DONE

           BREAK a
           SELECT SECOND
           OUTSTREAM STDERR
           WRITE       # a trailing comment
# A comment starts at the LAST hash-blank sequence on the line, so when a
# line holds more than one of them, everything before the last one is still
# parsed as spec units rather than skipped.

