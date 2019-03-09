import memcheck,sys

case_counter = 0

def run_case(spec, input, description):
    global case_counter
    case_counter = case_counter + 1
    (rc,info) = memcheck.leak_check_specs(spec,input)
    sys.stdout.write("Test case #{} - {} - ".format(case_counter,description))
    if rc!=memcheck.RetCode_SUCCESS:
        sys.stdout.write("Failed. RC={}; info={}\n".format(memcheck.RetCode_strings[rc],info))
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

