import memcheck,input_samples,sys

case_counter = 0

def run_case(spec, input, description, expected_rc=memcheck.RetCode_SUCCESS):
    global case_counter
    case_counter = case_counter + 1
    (rc,info) = memcheck.leak_check_specs(spec,input)
    sys.stdout.write("Test case #{} - {} - ".format(case_counter,description))
    if rc!=expected_rc:
        sys.stdout.write("Failed. RC={}; info={}; expected: {}\n".format(memcheck.RetCode_strings[rc],info,memcheck.RetCode_strings[expected_rc]))
        exit(4)
    else:
        sys.stdout.write("No leaks\n")


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

s = 'a: w5 . set "#0+=a" eof print "#0" 1'
i = input_samples.ls_out
run_case(s,i,"Summing up a field in the input")

