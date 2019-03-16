import sys, memcheck

count_ALU_tests = 344
count_processing_tests = 101
count_token_tests = 14

tests_that_may_fail = [43,48,58,61,63,152,153,154,342]
for i in range(count_ALU_tests):
    cmd = "../exe/ALUUnitTest {}".format(i+1)
    (rc,info) = memcheck.leak_check(cmd)
    if rc==memcheck.RetCode_SUCCESS:
        if (i+1) in tests_that_may_fail:
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

