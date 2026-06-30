import sys, memcheck, argparse, platform

count_ALU_tests = 852
count_processing_tests = 277
count_token_tests = 17

# Parse the one command line options
parser = argparse.ArgumentParser()
parser.add_argument("--no_valgrind", dest="nvg", action="store_true", default=None,
					help="Don't run valgrind - just check if the command succeeds.")
parser.add_argument("--allow_fail", dest="okfail", action="store_true", default=None,
					help="Ignore when tests fails")
args = parser.parse_args()
if args.nvg==True:
	memcheck.no_valgrind = True
all_tests_may_fail = True if args.okfail else False

# Find machine type
arch = platform.machine()

tests_to_skip = []
tests_that_may_fail = [9, 43, 48, 61, 62, 63, 68, 69, 70, 71, 73, 74, 83, 151, 152, 153, 154, 188, 197, 198, 199, 211, 403, 404, 405, 406, 409, 411, 413, 414, 415, 416, 417, 418, 421, 423, 426, 428, 429, 430, 483, 737, 817] if arch=='x86_64' else []
tests_that_failed = []
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
                memcheck.cleanup_valgrind()
                if all_tests_may_fail:
                    sys.stdout.write("ALUUnitTest Test #{} - no leaks\n".format(i+1))
                else:
                    sys.stdout.write("ALUUnitTest Test #{} - no leaks (but it should have failed -- check out cmd.out)\n".format(i+1))
                    exit(0)
        else:
            sys.stdout.write("ALUUnitTest Test #{} - no leaks\n".format(i+1))
    elif rc==memcheck.RetCode_COMMAND_FAILED and (i+1) in tests_that_may_fail or all_tests_may_fail:
        with open("cmd.out", "r") as f:
            cmdout = f.read().strip()
        sys.stdout.write("ALUUnitTest Test #{} - no leaks (but test failed)\n{}\n".format(i+1, cmdout))
        tests_that_failed.append(i+1)
    else:
        sys.stdout.write("Command '{}' failed with return code '{}' and extra info {}\nSee file valgrind.out for details.\n".format(cmd,memcheck.RetCode_strings[rc], info))
        exit(0)
    memcheck.cleanup()
if len(tests_that_failed) > 0:
    sys.stdout.write("ALUUnitTest: {} tests failed: {}\n".format(len(tests_that_failed), tests_that_failed))


tests_that_may_fail = []
tests_that_failed = []
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
    elif rc==memcheck.RetCode_COMMAND_FAILED and (i+1) in tests_that_may_fail or all_tests_may_fail:
        with open("cmd.out", "r") as f:
            cmdout = f.read().strip()
        sys.stdout.write("TokenTest Test #{} - no leaks (but test failed)\n{}\n".format(i+1, cmdout))
        tests_that_failed.append(i+1)
    else:
        sys.stdout.write("Command '{}' failed with return code '{}' and extra info {}\nSee file valgrind.out for details.\n".format(cmd,memcheck.RetCode_strings[rc], info))
        exit(0)
    memcheck.cleanup()
if len(tests_that_failed) > 0:
    sys.stdout.write("TokenTest: {} tests failed: {}\n".format(len(tests_that_failed), tests_that_failed))


tests_that_may_fail = [73, 102, 103, 179] if arch=='x86_64' else []
tests_that_failed = []
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
                # exit(0)
        else:
            sys.stdout.write("ProcessingTest Test #{} - no leaks\n".format(i+1))
    elif rc==memcheck.RetCode_COMMAND_FAILED and (i+1) in tests_that_may_fail or all_tests_may_fail:
        with open("cmd.out", "r") as f:
            cmdout = f.read().strip()
        sys.stdout.write("ProcessingTest Test #{} - no leaks (but test failed)\n{}\n".format(i+1, cmdout))
        tests_that_failed.append(i+1)
    else:
        sys.stdout.write("Command '{}' failed with return code '{}' and extra info {}\nSee file valgrind.out for details.\n".format(cmd,memcheck.RetCode_strings[rc], info))
        exit(0)
    memcheck.cleanup()
if len(tests_that_failed) > 0:
    sys.stdout.write("ProcessingTest: {} tests failed: {}\n".format(len(tests_that_failed), tests_that_failed))
