import memcheck,input_samples,sys,argparse

case_counter = 0

tests_to_run = None

def run_case(spec, input, description, expected_rc=memcheck.RetCode_SUCCESS, conf=""):
    global case_counter, tests_to_run
    case_counter = case_counter + 1
    if tests_to_run is not None and str(case_counter) not in tests_to_run:
    	return
    (rc,info) = memcheck.leak_check_specs(spec,input,case_counter,conf)
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
   if len(b)==8 then
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

# Some functions

s = 'a: word 1 . print "abs(a)" 1'
i = "1\n-1\n0\n-9.83\n1234567890\n1234567890123456789012\n-987654321"
run_case(s,i,"Functions: abs")

s = 'a: word 1 . print "frombin(a)" 1'
i = "A\nABCD\nABCDEFGH\nQWERTYUIOP\n"
run_case(s,i,"Functions: frombin",memcheck.RetCode_COMMAND_FAILED) 
# Because of the invalid length lines: QWERTYUIOP and the blank line

s = 'a: word 1 . b: word 2 . print "pow(a,b)" 1'
i = "1 0\n2 2\n5 9\n3.14 9.2\n98765.4 1234.8"
run_case(s,i,"Functions: pow")

s = 'a: word 1 . print "sqrt(a)" 1'
i = "1\n-1\n0\n-9.83\n1234567890\n1234567890123456789012\n-987654321\nAA"
run_case(s,i,"Functions: sqrt")

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

s = "a: word 1 . print 'len(a)' 1"
i = "the\nred\n\nincredible"
run_case(s,i,"Functions: len")

s = "a: word 1 . print 'substr(a,4,3)' 1"
i = "the\nincredible\n\nhulk"
run_case(s,i,"Functions: substr")

s = "a: word 1 . print 'pos(\"red\",a)' 1"
i = "the\nred\nincredible"
run_case(s,i,"Functions: pos")

s = "a: word 1 . print 'rpos(\"red\",a)' 1"
i = "the\nred\nincredible"
run_case(s,i,"Functions: rpos")




