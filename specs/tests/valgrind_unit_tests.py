import sys, memcheck, argparse

count_ALU_tests = 569
count_processing_tests = 135
count_token_tests = 14

# Parse the one command line options
parser = argparse.ArgumentParser()
parser.add_argument("--no_valgrind", dest="nvg", action="store_true", default=None,
					help="Don't run valgrind - just check if the command succeeds.")
args = parser.parse_args()
if args.nvg==True:
	memcheck.no_valgrind = True

tests_to_skip = [529]
tests_that_may_fail = [43,48,61,63,357,359,369,371]
for i in range(count_ALU_tests):
    if (i+1) in tests_to_skip:
        continue
    cmd = "../exe/ALUUnitTest {}".format(i+1)
    (rc,info) = memcheck.leak_check(cmd)
    if rc==memcheck.RetCode_SUCCESS:
        if (i+1) in tests_that_may_fail:
            if args.nvg:
                sys.stdout.write("ALUUnitTest Test #{} - valgrind not run, but it would have failed\n".format(i+1))
                memcheck.cleanup_valgrind()
            else:
                sys.stdout.write("ALUUnitTest Test #{} - no leaks (but it should have failed -- check out cmd.out)\n".format(i+1))
                memcheck.cleanup_valgrind()
                exit(0)
        else:
            sys.stdout.write("ALUUnitTest Test #{} - no leaks\n".format(i+1))
    elif rc==memcheck.RetCode_COMMAND_FAILED and (i+1) in tests_that_may_fail:
        sys.stdout.write("ALUUnitTest Test #{} - no leaks (but test failed)\n".format(i+1))
    else:
        sys.stdout.write("Command '{}' failed with return code '{}' and extra info {}\nSee file valgrind.out for details.\n".format(cmd,memcheck.RetCode_strings[rc], info))
        exit(0)
    memcheck.cleanup()


tests_that_may_fail = []
for i in range(count_token_tests):
    cmd = "../exe/TokenTest {}".format(i+1)
    (rc,info) = memcheck.leak_check(cmd)
    if rc==memcheck.RetCode_SUCCESS:
        if (i+1) in tests_that_may_fail:
            if args.nvg:
                sys.stdout.write("TokenTest Test #{} - valgrind not run, but it would have failed\n".format(i+1))
                memcheck.cleanup_valgrind()
            else:
                sys.stdout.write("TokenTest Test #{} - no leaks (but it should have failed -- check out cmd.out)\n".format(i+1))
                memcheck.cleanup_valgrind()
                exit(0)
        else:
            sys.stdout.write("TokenTest Test #{} - no leaks\n".format(i+1))
    elif rc==memcheck.RetCode_COMMAND_FAILED and (i+1) in tests_that_may_fail:
        sys.stdout.write("TokenTest Test #{} - no leaks (but test failed)\n".format(i+1))
    else:
        sys.stdout.write("Command '{}' failed with return code '{}' and extra info {}\nSee file valgrind.out for details.\n".format(cmd,memcheck.RetCode_strings[rc], info))
        exit(0)
    memcheck.cleanup()


tests_that_may_fail = [73]
for i in range(count_processing_tests):
    cmd = "../exe/ProcessingTest {}".format(i+1)
    (rc,info) = memcheck.leak_check(cmd)
    if rc==memcheck.RetCode_SUCCESS:
        if (i+1) in tests_that_may_fail:
            if args.nvg:
                sys.stdout.write("ProcessingTest Test #{} - valgrind not run, but it would have failed\n".format(i+1))
                memcheck.cleanup_valgrind()
            else:
                sys.stdout.write("ProcessingTest Test #{} - no leaks (but it should have failed -- check out cmd.out)\n".format(i+1))
                memcheck.cleanup_valgrind()
                exit(0)
        else:
            sys.stdout.write("ProcessingTest Test #{} - no leaks\n".format(i+1))
    elif rc==memcheck.RetCode_COMMAND_FAILED and (i+1) in tests_that_may_fail:
        sys.stdout.write("ProcessingTest Test #{} - no leaks (but test failed)\n".format(i+1))
    else:
        sys.stdout.write("Command '{}' failed with return code '{}' and extra info {}\nSee file valgrind.out for details.\n".format(cmd,memcheck.RetCode_strings[rc], info))
        exit(0)
    memcheck.cleanup()

