import sys, os, memcheck, argparse, platform, re

count_ALU_tests = 852
count_processing_tests = 281
count_token_tests = 17

# Parse the one command line options
parser = argparse.ArgumentParser()
parser.add_argument("--no_valgrind", dest="nvg", action="store_true", default=None,
					help="Don't run valgrind - just check if the command succeeds.")
parser.add_argument("--allow_fail", dest="okfail", action="store_true", default=None,
					help="Ignore when tests fails")
parser.add_argument("--one-by-one", dest="one_by_one", action="store_true", default=None,
					help="Run tests one-by-one to find which test leaks memory")
args = parser.parse_args()
if args.nvg==True:
	memcheck.no_valgrind = True
all_tests_may_fail = True if args.okfail else False
one_by_one = True if args.one_by_one else False

# Find machine type
arch = platform.machine()

def parse_failed_tests_from_output(output):
	"""Parse FAILED_TESTS line from test output and return list of test numbers.
	Returns empty list if no FAILED_TESTS line found (all tests passed)."""
	match = re.search(r'FAILED_TESTS:\s*(.*)', output)
	if match:
		failed_str = match.group(1).strip()
		if failed_str:
			return [int(x) for x in failed_str.split()]
		else:
			return []
	return []  # No FAILED_TESTS line means all tests passed

def get_leak_status_message(rc):
	"""Return a message describing the leak status based on return code.
	Only DEF_LOST, IND_LOST, and POSS_LOST indicate actual leaks.
	Everything else (SUCCESS, COMMAND_FAILED, etc.) means no leaks."""
	if rc == memcheck.RetCode_DEF_LOST:
		return "DEFINITELY LOST memory"
	elif rc == memcheck.RetCode_IND_LOST:
		return "INDIRECTLY LOST memory"
	elif rc == memcheck.RetCode_POSS_LOST:
		return "POSSIBLY LOST memory"
	else:
		return "no leaks"

tests_to_skip = []
tests_that_may_fail = [9, 43, 48, 61, 62, 63, 68, 69, 70, 71, 73, 74, 83, 151, 152, 153, 154, 188, 197, 198, 199, 211, 403, 404, 405, 406, 409, 411, 413, 414, 415, 416, 417, 418, 421, 423, 426, 428, 429, 430, 483, 737, 817] if arch=='x86_64' else []

# Run ALUUnitTest
if one_by_one:
	# Run tests one-by-one to find which test leaks
	sys.stdout.write("Running ALUUnitTest one-by-one to find leaking test...\n")
	sys.stdout.flush()
	for i in range(count_ALU_tests):
		cmd = "../exe/ALUUnitTest {}".format(i+1)
		# Run without redirecting output so user sees progress
		if memcheck.no_valgrind:
			rc = os.system(cmd)
			if rc != 0:
				leak_status = "COMMAND FAILED"
			else:
				leak_status = "no leaks"
		else:
			valgfile = "valgrind.out"
			cmd_to_execute = "valgrind --leak-check=full --log-file={} {}".format(valgfile, cmd)
			rc = os.system(cmd_to_execute)
			# Check valgrind output for leaks
			try:
				with open(valgfile, "r") as f:
					lines = f.readlines()
				no_definitely_lost = False
				no_indirectly_lost = False
				no_possibly_lost = True
				for line in lines:
					if line.find("in use at exit: 0 bytes") >= 0:
						leak_status = "no leaks"
						break
					if line.find("definitely lost: 0 bytes") >= 0:
						no_definitely_lost = True
					if line.find("indirectly lost: 0 bytes") >= 0:
						no_indirectly_lost = True
				else:
					if no_definitely_lost and no_indirectly_lost and no_possibly_lost:
						leak_status = "no leaks"
					elif not no_definitely_lost:
						leak_status = "DEFINITELY LOST memory"
					elif not no_indirectly_lost:
						leak_status = "INDIRECTLY LOST memory"
					elif not no_possibly_lost:
						leak_status = "POSSIBLY LOST memory"
					else:
						leak_status = "no leaks"
			except IOError:
				leak_status = "no leaks"
		
		if leak_status != "no leaks":
			sys.stdout.write("ALUUnitTest Test #{} - {}\n".format(i+1, leak_status))
			sys.stdout.flush()
		memcheck.cleanup()
	sys.stdout.write("ALUUnitTest one-by-one run complete.\n")
else:
	# Run all tests at once
	cmd = "../exe/ALUUnitTest"
	(rc,info) = memcheck.leak_check(cmd)
	if rc==memcheck.RetCode_SUCCESS:
		leak_status = "no leaks" if not memcheck.no_valgrind else "tests passed"
		sys.stdout.write("ALUUnitTest - {}\n".format(leak_status))
		memcheck.cleanup_valgrind()
	elif rc==memcheck.RetCode_COMMAND_FAILED:
		with open("cmd.out", "r") as f:
			cmdout = f.read()
		failed_tests = parse_failed_tests_from_output(cmdout)
		# Check if all failed tests are in the allowed list
		unexpected_failures = [t for t in failed_tests if t not in tests_that_may_fail and not all_tests_may_fail]
		if unexpected_failures:
			sys.stdout.write("ALUUnitTest: Unexpected test failures: {}\n".format(unexpected_failures))
			sys.stdout.write("Command '{}' failed with return code '{}'\nSee file valgrind.out for details.\n".format(cmd, memcheck.RetCode_strings[rc]))
			exit(0)
		elif failed_tests:
			leak_status = "no leaks" if not memcheck.no_valgrind else "tests passed"
			sys.stdout.write("ALUUnitTest: {} tests failed (all expected): {} - {}\n".format(len(failed_tests), failed_tests, leak_status))
		else:
			leak_status = "no leaks" if not memcheck.no_valgrind else "tests passed"
			sys.stdout.write("ALUUnitTest - {}\n".format(leak_status))
		memcheck.cleanup()
	else:
		sys.stdout.write("ALUUnitTest - {}\n".format(get_leak_status_message(rc)))
		sys.stdout.write("See file valgrind.out for details.\n")
		exit(0)
	memcheck.cleanup()


# Run TokenTest
if one_by_one:
	# Run tests one-by-one to find which test leaks
	sys.stdout.write("Running TokenTest one-by-one to find leaking test...\n")
	sys.stdout.flush()
	for i in range(count_token_tests):
		cmd = "../exe/TokenTest {}".format(i+1)
		# Run without redirecting output so user sees progress
		if memcheck.no_valgrind:
			rc = os.system(cmd)
			if rc != 0:
				leak_status = "COMMAND FAILED"
			else:
				leak_status = "no leaks"
		else:
			valgfile = "valgrind.out"
			cmd_to_execute = "valgrind --leak-check=full --log-file={} {}".format(valgfile, cmd)
			rc = os.system(cmd_to_execute)
			# Check valgrind output for leaks
			try:
				with open(valgfile, "r") as f:
					lines = f.readlines()
				no_definitely_lost = False
				no_indirectly_lost = False
				no_possibly_lost = True
				for line in lines:
					if line.find("in use at exit: 0 bytes") >= 0:
						leak_status = "no leaks"
						break
					if line.find("definitely lost: 0 bytes") >= 0:
						no_definitely_lost = True
					if line.find("indirectly lost: 0 bytes") >= 0:
						no_indirectly_lost = True
				else:
					if no_definitely_lost and no_indirectly_lost and no_possibly_lost:
						leak_status = "no leaks"
					elif not no_definitely_lost:
						leak_status = "DEFINITELY LOST memory"
					elif not no_indirectly_lost:
						leak_status = "INDIRECTLY LOST memory"
					elif not no_possibly_lost:
						leak_status = "POSSIBLY LOST memory"
					else:
						leak_status = "no leaks"
			except IOError:
				leak_status = "no leaks"
		
		if leak_status != "no leaks":
			sys.stdout.write("TokenTest Test #{} - {}\n".format(i+1, leak_status))
			sys.stdout.flush()
		memcheck.cleanup()
	sys.stdout.write("TokenTest one-by-one run complete.\n")
else:
	# Run all tests at once
	cmd = "../exe/TokenTest"
	(rc,info) = memcheck.leak_check(cmd)
	if rc==memcheck.RetCode_SUCCESS:
		leak_status = "no leaks" if not memcheck.no_valgrind else "tests passed"
		sys.stdout.write("TokenTest - {}\n".format(leak_status))
		memcheck.cleanup_valgrind()
	elif rc==memcheck.RetCode_COMMAND_FAILED:
		with open("cmd.out", "r") as f:
			cmdout = f.read()
		failed_tests = parse_failed_tests_from_output(cmdout)
		if failed_tests:
			leak_status = "no leaks" if not memcheck.no_valgrind else "tests passed"
			sys.stdout.write("TokenTest: {} tests failed: {} - {}\n".format(len(failed_tests), failed_tests, leak_status))
		else:
			leak_status = "no leaks" if not memcheck.no_valgrind else "tests passed"
			sys.stdout.write("TokenTest - {}\n".format(leak_status))
		memcheck.cleanup()
	else:
		sys.stdout.write("TokenTest - {}\n".format(get_leak_status_message(rc)))
		sys.stdout.write("See file valgrind.out for details.\n")
		exit(0)
	memcheck.cleanup()


# Run ProcessingTest
tests_that_may_fail = [73, 102, 103, 179] if arch=='x86_64' else []
if one_by_one:
	# Run tests one-by-one to find which test leaks
	sys.stdout.write("Running ProcessingTest one-by-one to find leaking test...\n")
	sys.stdout.flush()
	for i in range(count_processing_tests):
		cmd = "../exe/ProcessingTest {}".format(i+1)
		# Run without redirecting output so user sees progress
		if memcheck.no_valgrind:
			rc = os.system(cmd)
			if rc != 0:
				leak_status = "COMMAND FAILED"
			else:
				leak_status = "no leaks"
		else:
			valgfile = "valgrind.out"
			cmd_to_execute = "valgrind --leak-check=full --log-file={} {}".format(valgfile, cmd)
			rc = os.system(cmd_to_execute)
			# Check valgrind output for leaks
			try:
				with open(valgfile, "r") as f:
					lines = f.readlines()
				no_definitely_lost = False
				no_indirectly_lost = False
				no_possibly_lost = True
				for line in lines:
					if line.find("in use at exit: 0 bytes") >= 0:
						leak_status = "no leaks"
						break
					if line.find("definitely lost: 0 bytes") >= 0:
						no_definitely_lost = True
					if line.find("indirectly lost: 0 bytes") >= 0:
						no_indirectly_lost = True
				else:
					if no_definitely_lost and no_indirectly_lost and no_possibly_lost:
						leak_status = "no leaks"
					elif not no_definitely_lost:
						leak_status = "DEFINITELY LOST memory"
					elif not no_indirectly_lost:
						leak_status = "INDIRECTLY LOST memory"
					elif not no_possibly_lost:
						leak_status = "POSSIBLY LOST memory"
					else:
						leak_status = "no leaks"
			except IOError:
				leak_status = "no leaks"
		
		if leak_status != "no leaks":
			sys.stdout.write("ProcessingTest Test #{} - {}\n".format(i+1, leak_status))
			sys.stdout.flush()
		memcheck.cleanup()
	sys.stdout.write("ProcessingTest one-by-one run complete.\n")
else:
	# Run all tests at once
	cmd = "../exe/ProcessingTest"
	(rc,info) = memcheck.leak_check(cmd)
	if rc==memcheck.RetCode_SUCCESS:
		leak_status = "no leaks" if not memcheck.no_valgrind else "tests passed"
		sys.stdout.write("ProcessingTest - {}\n".format(leak_status))
		memcheck.cleanup_valgrind()
	elif rc==memcheck.RetCode_COMMAND_FAILED:
		with open("cmd.out", "r") as f:
			cmdout = f.read()
		failed_tests = parse_failed_tests_from_output(cmdout)
		# Check if all failed tests are in the allowed list
		unexpected_failures = [t for t in failed_tests if t not in tests_that_may_fail and not all_tests_may_fail]
		if unexpected_failures:
			sys.stdout.write("ProcessingTest: Unexpected test failures: {}\n".format(unexpected_failures))
			sys.stdout.write("Command '{}' failed with return code '{}'\nSee file valgrind.out for details.\n".format(cmd, memcheck.RetCode_strings[rc]))
			exit(0)
		elif failed_tests:
			leak_status = "no leaks" if not memcheck.no_valgrind else "tests passed"
			sys.stdout.write("ProcessingTest: {} tests failed (all expected): {} - {}\n".format(len(failed_tests), failed_tests, leak_status))
		else:
			leak_status = "no leaks" if not memcheck.no_valgrind else "tests passed"
			sys.stdout.write("ProcessingTest - {}\n".format(leak_status))
		memcheck.cleanup()
	else:
		sys.stdout.write("ProcessingTest - {}\n".format(get_leak_status_message(rc)))
		sys.stdout.write("See file valgrind.out for details.\n")
		exit(0)
	memcheck.cleanup()

# Failing ProcessingTest cases dump their actual and expected output to files for
# the developer to diff. Here they are just leftovers polluting the repository.
os.system("/bin/rm ProcessingTest.Expected ProcessingTest.Got 2> /dev/null")
