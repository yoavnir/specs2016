import memcheck,input_samples,sys,argparse

case_counter = 0

tests_to_run = None

def run_case(spec, input, description, expected_rc=memcheck.RetCode_SUCCESS, conf="", inp2=None):
    global case_counter, tests_to_run
    case_counter = case_counter + 1
    if tests_to_run is not None and str(case_counter) not in tests_to_run:
    	return
    (rc,info) = memcheck.leak_check_specs(spec,input,case_counter,conf,inp2)
    sys.stdout.write("Test case #{} - {} - ".format(case_counter,description))
    if rc!=expected_rc:
        sys.stdout.write("Failed. RC={}; info={}; expected: {}\n".format(memcheck.RetCode_strings[rc],info,memcheck.RetCode_strings[expected_rc]))
        exit(4)
    else:
        sys.stdout.write("No leaks\n")
        
# Parse the one command line options
parser = argparse.ArgumentParser()
parser.add_argument("--no_valgrind", dest="nvg", action="store_true", default=None,
					help="Don't run valgrind - just check if the command succeeds.")
parser.add_argument("--keep_output", dest="keep", action="store_true", default=None,
					help="Keep output files.")
parser.add_argument("--only", dest="only", action="store", default="", 
					help="Comma-separated list of test numbers")
args = parser.parse_args()
if args.nvg==True:
	memcheck.no_valgrind = True
if args.keep==True:
	memcheck.keep_specs_output = True
if args.only!="":
	tests_to_run = args.only.split(",")


# Now come the test cases

s = "hello 1"
i = None
run_case(s,i,"Literal in first column")

s = "2 1"
i = "hello"
run_case(s,i,"Random char in first column")

s = "w2 5"
i = input_samples.Jabberwocky
run_case(s,i,"Random word in random column")

s = "w2 ucase 5"
i = input_samples.Jabberwocky
run_case(s,i,"Simple conversion")

s = "w2 c2x 5"
i = input_samples.Jabberwocky
run_case(s,i,"Another conversion")

s = "w2 x2ch 5"
i = input_samples.Jabberwocky
run_case(s,i,"A conversion with errors",memcheck.RetCode_COMMAND_FAILED) # Because the words of Jabberwocky are not hex

s = "1-7 3 w2 6 w2:-2 19"
i = input_samples.Jabberwocky
run_case(s,i,"Some overlapping outputs")

s = 'a: w5 . set "#0+=a" eof print "#0" 1'
i = input_samples.ls_out
run_case(s,i,"Summing up a field in the input")

s = "substr 2:-2 of w3 8.10 right"
i = input_samples.Jabberwocky
run_case(s,i,"alignment and substr")

s = "substr 2:-2 of w3 8.10 center /<<</ n"
i = input_samples.Jabberwocky
run_case(s,i,"center-alignment and substr")

s = "substr 2:-2 of w3 8.20 centre /<<</ n"
i = input_samples.Jabberwocky
run_case(s,i,"centre-alignment and substr")

s = \
'''
   2.4 c2x b: 
   if length(b)==8 then
       id b x2d a: 
       set "#0+=a" 
   endif
   eof 
       print "#0" 1
'''
i = input_samples.Jabberwocky
run_case(s,i,"result field identifier and conversion")

s = 'a: NUMBER 1'
i = input_samples.ls_out
run_case(s,i,"Unused field identifier")

s = "fs , f1 1 f3 nw f5 nf"
i = "1,2,3,4,5,6\n7,8,9,10\n11,12,13,14,15,16"
run_case(s,i,"Fields with one line not having enough fields")

# Some more, weird input sources

s = "TODclock 1 w1 nw"
i = "Hope\nis\nthe\nthing\nwith\nfeathers\nthat\nperches\nin\nthe\nsoul"
run_case(s,i,"TODclock")

s = "DTODclock 1 w1 nw"
i = "Hope\nis\nthe\nthing\nwith\nfeathers\nthat\nperches\nin\nthe\nsoul"
run_case(s,i,"DTODclock")

s = "TIMEDIFF 1 w1 nw"
i = "Hope\nis\nthe\nthing\nwith\nfeathers\nthat\nperches\nin\nthe\nsoul"
run_case(s,i,"TIMEDIFF")

s = "NUMBER 1 w1 nw"
i = "Hope\nis\nthe\nthing\nwith\nfeathers\nthat\nperches\nin\nthe\nsoul"
run_case(s,i,"NUMBER")

# more fun with substr

s = "substr fs , f3 of w2 1"
i = "Let t,h,e,r,e be light\nAnd there was light"
run_case(s,i,"SUBSTR with field separator")

s = "fs ' ' substr ws , w3 of f2 1"
i = "Let t,h,e,r,e be light\nAnd there was light"
run_case(s,i,"SUBSTR with word separator")

# Some basic ALU stuff

s = 'print "\'2+3=\' || (2+3)" 1'
i = None
run_case(s,i,"Simple ALU addition and concatenation")

s = 'a: word 1 .  PRINT "\'The first word is \' || a" 1'
i = "Hello, there\nMy, it's been a long long time"
run_case(s,i,"Concatenation with field identifier")

s = 'a: word 1 1  "items. Total is" nextword  SET "#0+=a"  PRINT "#0" nextword'
i = "5\n7\n3"
run_case(s,i,"Counter summing up a field identifier")

s = \
'''
r: word 1 .
   /Tau is/ 1
   PRINT "@pi*2" nextword
   /; Circle area is/ next
   PRINT "@pi*r*r" nextword
   /; My favorite animal is a/ next
   PRINT "@favoriteAnimal" nextword
   /; My motto is:/ next
   PRINT "@Motto" nextword
'''
i = "5\n7\n3"
c = \
'''
pi: 3.14159265
favoriteAnimal: cat
billion: 1000000000
Motto: "memento mori"
'''
run_case(s,i,"Fun with configured values",conf=c)

s = "print '@version' 1 print '@@' nw"
i = "cat"
run_case(s,i,"entire line and version")

# Some operator tests

s = "a: word 1 . print '!a' 1 print '+a' nw print '-a' nw"
i = "0\n1\n2"
run_case(s,i,"Unary Operators")

s = \
"""
a: word 1 . b: word 2 . c: word 3 . d: word 4 .
   print 'a+b-c*d/6' 1
   print 'a//b' nw
   print 'a%b' nw
   print 'a||b' nw
   write
   print 'a<b' 1
   print 'a<=b' nw
   print 'a>b' nw
   print 'a>=b' nw
   print 'a<<b' nw
   print 'c>>d' nw
   print 'a<<=c' nw
   print 'b>>=d' nw
   print 'a=d' nw
   print 'b==c' nw
   print 'c!=a' nw
   print 'd!==b' nw
   print 'a>b & c<d' nw
   print 'a>b | c<d' nw
"""
i = "1 2 3 4\n9 8 7 6"
run_case(s,i,"Binary Operators")

s = \
'''
a: word 1 .
   set '#0:=a'
   set '#1+=a'
   set '#2-=a'
   set '#3*=a'
   set '#4/=a'
   set '#5//=a'
   set '#6%=a'
   set '#7||=a'
   print '#0 + #1 + #2 + #3 + #4 + #5 + #6 + #7' 1
'''
i = "1\n2\n3\n4\n1\n2\n3\n4"
run_case(s,i,"Assignment Operators")

s = "a: word 1 . print '#0:=a' 1 print '#1+=a' nw"
i = "1\n2\n3\n4"
run_case(s,i,"Assignments as Expressions")

s = "w1 1 @version nw"
i = "1\n2\n3\n4"
run_case(s,i,"Configured strings - No errors")

s = "w1 1 @version nw @kuku nw"
i = "1\n2\n3\n4"
run_case(s,i,"Configured strings - With errors")

# Some functions

s = 'a: word 1 . print "abs(a)" 1'
i = "1\n-1\n0\n-9.83\n1234567890\n1234567890123456789012\n-987654321"
run_case(s,i,"Functions: abs",memcheck.RetCode_SUCCESS)

s = 'a: word 1 . print "frombin(a)" 1'
i = "A\nABCD\nABCDEFGH\nQWERTYUIOP\n"
run_case(s,i,"Functions: frombin",memcheck.RetCode_COMMAND_FAILED) 
# Because of the invalid length lines: QWERTYUIOP and the blank line

s = 'a: word 1 . b: word 2 . print "pow(a,b)" 1'
i = "1 0\n2 2\n5 9\n3.14 9.2\n98765.4 1234.8"
run_case(s,i,"Functions: pow")

s = 'a: word 1 . print "sqrt(a)" 1'
i = "1\n-1\n0\n-9.83\n1234567890\n1234567890123456789012\n-987654321\nAA"
run_case(s,i,"Functions: sqrt",memcheck.RetCode_SUCCESS)

s = 'a: word 1 . print "tobin(a)" 1'
i = "1\n-1\n0\n1234567890\n1234567890123456789012\n-987654321\n9.83\n"
run_case(s,i,"Functions: tobin",memcheck.RetCode_COMMAND_FAILED)
# out of range exception is stoll converting big number to int

s = 'a: word 1 . b: word 2 . print "tobine(a,b)" 1'
i = "1 8\n-1 8\n0 8\n1234567890 32\n1234567890123456789012 64\n-987654321 32\n9.83 32\n"
run_case(s,i,"Functions: tobine",memcheck.RetCode_COMMAND_FAILED)
# same

s = "a: word 1 . print 'center(a,8)' 1"
i = "the\nincredible\nhulk"
run_case(s,i,"Functions: center")

s = "a: word 1 . print 'left(a,8)' 1"
i = "the\nincredible\nhulk"
run_case(s,i,"Functions: left")

s = "a: word 1 . print 'right(a,8)' 1"
i = "the\nincredible\nhulk"
run_case(s,i,"Functions: right")

s = "a: word 1 . print 'includes(a,\"red\")' 1"
i = "the\nred\nincredible"
run_case(s,i,"Functions: includes")

s = "a: word 1 . print 'includes(a,\"red\",\"incredible\")' 1"
i = "the\nred\nincredible\nthe incredible red"
run_case(s,i,"Functions: includes (multiple needles)")

s = "a: word 1 . print 'includesall(a,\"red\")' 1"
i = "the\nred\nincredible"
run_case(s,i,"Functions: includesall")

s = "a: word 1 . print 'includesall(a,\"red\",\"incredible\")' 1"
i = "the\nred\nincredible\nthe incredible red"
run_case(s,i,"Functions: includesall (multiple needles)")

s = "a: word 1 . print 'length(a)' 1"
i = "the\nred\n\nincredible"
run_case(s,i,"Functions: length")

s = "a: word 1 . print 'substr(a,4,3)' 1"
i = "the\nincredible\n\nhulk"
run_case(s,i,"Functions: substr")

s = "a: word 1 . print 'pos(\"red\",a)' 1"
i = "the\nred\nincredible"
run_case(s,i,"Functions: pos")

s = "a: word 1 . print 'lastpos(\"red\",a)' 1"
i = "the\nred\nincredible"
run_case(s,i,"Functions: lastpos")

# Record Access Functions

s = "print 'field(3)' 1"
i = "a\tb\tc\td \n a\tb \n \t\t\t\t"
run_case(s,i,"Functions: field")

s = "print 'fieldrange(2,3)' 1"
i = "a\tb\tc\td \n a\tb \n \t\t\t\t"
run_case(s,i,"Functions: fields")

s = "--force-read-input print 'fieldcount()' 1"
i = "a\tb\tc\td \n a\tb \n \t\t\t\t\n"
run_case(s,i,"Functions: fieldcount")

s = "print 'fieldend(3)' 1"
i = "a\tb\tc\td \n a\tb \n \t\t\t\t"
run_case(s,i,"Functions: fieldend")

s = "print 'fieldindex(3)' 1"
i = "a\tb\tc\td \n a\tb \n \t\t\t\t"
run_case(s,i,"Functions: fieldindex")

s = "print 'fieldlength(3)' 1"
i = "a\tb\tc\td \n a\tb \n \t\t\t\t"
run_case(s,i,"Functions: fieldlength")

s = "print 'number()' 1 print 'recno()' nw"
i = "1\n2\n3\n4\n5\n6\n7\n8\n9\n10"
run_case(s,i,"Functions: number & recno (1)")

s = "print 'number()' 1 print 'recno()' nw READ print 'number()' nw print 'recno()' nw"
i = "1\n2\n3\n4\n5\n6\n7\n8\n9"
run_case(s,i,"Functions: number & recno (2)",memcheck.RetCode_COMMAND_FAILED)

s = "print 'number()' 1 print 'recno()' nw READSTOP print 'number()' nw print 'recno()' nw"
i = "1\n2\n3\n4\n5\n6\n7\n8\n9"
run_case(s,i,"Functions: number & recno (3)",memcheck.RetCode_COMMAND_FAILED)

s = "print 'record()' 1"
i = "1\n\nhello"
run_case(s,i,"Functions: record")

s = "print 'word(3)' 1"
i = "Hope is the thing\n with feathers\n\n\n"
run_case(s,i,"Functions: word")

s = "print 'wordrange(2,3)' 1"
i = "Hope is the thing\n with feathers\n\n\n"
run_case(s,i,"Functions: wordrange")

s = "--force-read-input print 'wordcount()' 1"
i = "Hope is the thing\n with feathers\n\n\n"
run_case(s,i,"Functions: wordcount")

s = "print 'wordend(3)' 1"
i = "Hope is the thing\n with feathers\n\n\n"
run_case(s,i,"Functions: wordend")

s = "print 'wordstart(3)' 1"
i = "Hope is the thing\n with feathers\n\n\n"
run_case(s,i,"Functions: wordstart")

s = "print 'wordlen(3)' 1"
i = "Hope is the thing\n with feathers\n\n\n"
run_case(s,i,"Functions: wordlen")

# Special functions

s = "print 'first()' 1"
i = None
run_case(s,i,"Functions: first(1) - nor reading lines")

s = "print 'first()' 1 w1 nw"
i = "a\nb\nc\nd"
run_case(s,i,"Functions: first(2) - reading lines")

s = \
"""
if 'first()' then 
	w1 1
else
	w2 1
endif
"""
i = "a\nb\nc\nd"
run_case(s,i,"Functions: first(3) - reading lines and branching on the result")

s = "print 'eof()' 1"
i = None
run_case(s,i,"Functions: eof(1) - nor reading lines")

s = "print 'eof()' 1 w1 nw"
i = "a\nb\nc\nd"
run_case(s,i,"Functions: eof(2) - reading lines")

s = \
"""
if 'eof()' then 
	w1 1
else
	w2 1
endif
"""
i = "a\nb\nc\nd"
run_case(s,i,"Functions: eof(3) - reading lines and branching on the result")

s = "print 'conf(\"version\")' 1 print 'conf(\"cracker\")' nw"
i = None
run_case(s,i,"Functions: conf")

s = "a: w1 . print 's2tf(a,\"%Y-%m-%dT%H:%M:%S\")' 1"
i = "1553109338\n1553109340\n15\n-1953109338"
run_case(s,i,"Functions: s2tf(1)")

s = "a: w1 . print 's2tf(a,\"%Y-%m-%dT%H:%M:%S.%6f\")' 1"
i = "1553109338\n1553109340.15\n15.9876\n-1953109338.123456"
run_case(s,i,"Functions: s2tf(2)")

s = "print 'tf2s(@@,\"%Y-%m-%dT%H:%M:%S\")' 1"
i = "2024-12-05T23:45:12"
run_case(s,i,"Functions: tf2s(1)")

s = "print 'tf2s(@@,\"%H:%M:%S.%6f\")' 1"
i = "23:45:12\n12:12:76.743\n13:13:13.123456"
run_case(s,i,"Functions: tf2s(1)")

# Conditional Execution

s = \
'''
a: w1 1 /is/ nw
    if "a%2" then
        /odd/ nw
    else
        /even/ nw
    endif
'''
i = "0\n1\n25\n404\n2.5\n3.5\nhello"
run_case(s,i,"Conditional Execution (1) - if-then-else-endif")

s = \
'''
a: w1 1 /is/ nw
    if "a%2" then
        /odd/ nw
    else
        /even/ nw
'''
i = "0\n1\n25\n404\n2.5\n3.5\nhello"
run_case(s,i,"Conditional Execution (2) - if-then-else; missing endif")

s = \
'''
a: w1 .
    if "0==a%15" then
        /fizzbuzz/ 1
    elseif "0==a%5" then
        /buzz/ 1
    elseif "0==a%3" then
        /fizz/ 1
    else 
        ID a 1
    endif
'''
i = "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20"
run_case(s,i,"Conditional Execution (3) - fizzbuzz")

s = "a: w1 1 /is/ nw if 'a%2' then /not/ nw endif /even/ nw"
i = "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20"
run_case(s,i,"Conditional Execution (4) - no else clause")

s = "a: w1 1 /is/ nw if '0==a%8' then /very much not/ nw elseif 'a%2' then /not/ nw endif /even/ nw"
i = "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20"
run_case(s,i,"Conditional Execution (5) - elseif, but no else clause")

s = \
"""
a: w1 1 /is/ nw 
	if "a==1 | a==4 | a==9 | a==16 | a==25" then
		/a square/ nw
	else
		/probably not a square/ nw
	endif
"""
i = "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20"
run_case(s,i,"Conditional Execution (6) - complex condition")

# loops
s = \
"""
a: w1 1.3 right / / n
	set '#0:=a'
	while '#0>0' do
		/*/ n
		set '#0-=1'
	done
"""
i = "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20"
run_case(s,i,"While Loop (1) - simple counter")

s = \
"""
while '#1<20' do
	set "#1+=1"
	set '#0:=#1'
	while '#0>0' do
		/*/ n
		set '#0-=1'
	done
done
"""
i = None
run_case(s,i,"While Loop (2) - same as (1) but no input")

# EOF

s = \
"""
    1-*    1
 a: word 1 .
    set '#0+=a'
 EOF
 	/Total:/ 1
 	print #0 NW
"""
i = "1\n2\n3\n4"
run_case(s,i,"Run-Out Cycle with EOF")

# Control Break

s = \
'''
	FIELDSEPARATOR ,
 c: FIELD 1   .
	FIELD 3  10
	/,/      NEXT
	FIELD 2  NEXTWORD
	BREAK c
		ID c 1
'''
i = input_samples.employees
run_case(s,i,"Control Break (1) - with BREAK keyword")

s = \
'''
	FIELDSEPARATOR ,
 a: FIELD 1    .
	IF break(a) THEN
		ID a           1
		/Department:/ NW
			WRITE
	ENDIF
	FIELD 3   10
	,          N
	FIELD 2   NW
'''
i = input_samples.employees
run_case(s,i,"Control Break (2) - with break() function")

# WRITE

s = \
'''
/Filename:/ 1
word -1     nw
WRITE
/type:/ 1
if "range(1,1)=='d'" then
	/directory/ nw
else
	/file/ nw
endif
'''
i = input_samples.ls_out
run_case(s,i,"WRITE")

# READ
s = \
"""
IF "first()" THEN
	READ  # Read away the first blank line from the input
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
"""
i = input_samples.gitlog
run_case(s,i,"READ and READSTOP",memcheck.RetCode_COMMAND_FAILED)

# UNREAD
s = \
"""
IF "first()" THEN
	READ  # Read away the first blank line from the input
ENDIF
WORD 2                    1
READSTOP
WORD 2             NEXTWORD
READSTOP
WORD 2-6 tf2s "%c" NEXTWORD
WHILE "word(1)!='commit'" DO
	READSTOP
DONE
UNREAD
"""
i = input_samples.gitlog
run_case(s,i,"UNREAD",memcheck.RetCode_COMMAND_FAILED)

# REDO

s = "fs : f2-* 1 REDO /source:/ 1 w1 nw"
i = input_samples.httplog
run_case(s,i,"REDO")

# Second Reading
s = \
'''
WORD 1        1
SELECT SECOND
WORD 1 NEXTWORD
SELECT FIRST
WORD 2 NEXTWORD
SELECT SECOND
WORD 2 NEXTWORD
'''
i = "first record\nsecond line\nlast one"
run_case(s,i,"Second Reading")

# Dual stream
s = \
"""
WORD 9 1.25 LEFT
SELECT 2 
WORD 1 NEXTWORD
SELECT 1
WORD 5 NW.9 RIGHT
"""
i = input_samples.ls_out
i2 = input_samples.ls_out_inodes
run_case(s,i,"dual input stream",inp2=i2)

# Dual stream without specifying it
run_case(s,i,"dual input stream missing secondary",expected_rc=memcheck.RetCode_COMMAND_FAILED)

# Dual stream with mismatched streams
i2 = input_samples.ls_out_inodes_mismatched
run_case(s,i,"dual input stream with mismatched secondary",inp2=i2)

# Dual stream with mismatched streams - STOP ALLEOF
i2 = input_samples.ls_out_inodes_mismatched
run_case("STOP ALLEOF "+s,i,"dual input stream with mismatched secondary - STOP ALLEOF",inp2=i2)

# Dual stream with mismatched streams - STOP ANYEOF
i2 = input_samples.ls_out_inodes_mismatched
run_case("STOP ANYEOF "+s,i,"dual input stream with mismatched secondary - STOP ANYEOF",inp2=i2)

# Dual stream with mismatched streams - STOP 2
i2 = input_samples.ls_out_inodes_mismatched
run_case("STOP 2 "+s,i,"dual input stream with mismatched secondary - STOP 2",inp2=i2)

# Dual output stream 
i = input_samples.ls_out_inodes
s = \
"""
WORD 1 1
WRITE
OUTSTREAM STDERR
WORD 2 1 
"""
run_case(s,i,"dual output")

# NOWRITE
s = "WORD 1 1 NOWRITE"
i = "first record\nsecond line\nlast one"
run_case(s,i,"nowrite")

# ASSERT & ABEND
s = "a: WORD 1 1 ASSERT 'a<5'"
i = '1\n2\n1\n2'
run_case(s,i,"an assertion that always succeeds")

i = '1\n2\n3\n4\n5\n6\n7'
run_case(s,i,"an assertion that fails",expected_rc=memcheck.RetCode_COMMAND_FAILED)

s = "a: WORD 1 1 if 'a>=5' then abend 'oops' endif"
i = '1\n2\n1\n2'
run_case(s,i,"an abend that is never reached")

i = '1\n2\n3\n4\n5'
run_case(s,i,"an abend that is reached",expected_rc=memcheck.RetCode_COMMAND_FAILED)

# output formatting
s = "print \"fmt(sqrt(2),,10,',',':')\" 1"
run_case(s,i,"a formatted number")

# breaking out with continue
s = 'if "word(2)==\'mark\'" then continue endif w1 1 w3 nw'
i = input_samples.matrix_with_marks
run_case(s,i,"breaking out with continue #1")

s = 'w1 1 if "word(2)==\'mark\'" then continue endif w3 nw'
i = input_samples.matrix_with_marks
run_case(s,i,"breaking out with continue #2")

s = 'skip-while "wordcount()<5" w9'
i = input_samples.ls_out_hdr
run_case(s,i,"Skipping a line one way")

s = 'skip-until "wordcount()==9" w9'
i = input_samples.ls_out_hdr
run_case(s,i,"Skipping a line another way")

s = \
"""
+IN ls 
W! 
"""
run_case(s,i,"inline input")

s = \
"""
+SET fname ls
'The file name is' 1 w1 nw
"""
run_case(s,i,"inline variable")

