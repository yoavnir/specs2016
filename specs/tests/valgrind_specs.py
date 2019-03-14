import memcheck,input_samples,sys,argparse

case_counter = 0

def run_case(spec, input, description, expected_rc=memcheck.RetCode_SUCCESS):
    global case_counter
    case_counter = case_counter + 1
    (rc,info) = memcheck.leak_check_specs(spec,input,case_counter)
    sys.stdout.write("Test case #{} - {} - ".format(case_counter,description))
    if rc!=expected_rc:
        sys.stdout.write("Failed. RC={}; info={}; expected: {}\n".format(memcheck.RetCode_strings[rc],info,memcheck.RetCode_strings[expected_rc]))
        exit(4)
    else:
        sys.stdout.write("No leaks\n")
        
# Parse the one command line option - --no_valgrind

parser = argparse.ArgumentParser()
parser.add_argument("--no_valgrind", dest="nvg", action="store_true", default=None,
					help="Don't run valgrind - just check if the command succeeds.")
parser.add_argument("--keep_output", dest="keep", action="store_true", default=None,
					help="Keep output files.")
args = parser.parse_args()
if args.nvg==True:
	memcheck.no_valgrind = True
if args.keep==True:
	memcheck.keep_specs_output = True

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




