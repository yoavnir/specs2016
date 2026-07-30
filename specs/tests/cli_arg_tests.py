import os, sys, argparse, subprocess, shlex

# Test script for command-line argument validation

specs_exe = "../exe/specs"

case_counter = 0
fail_counter = 0
tests_to_run = None

specfile = "./cli_spec"
inpfile = "cli_inp"
outfile = "cli_out"
errfile = "cli_err"

def read_file(path):
    if not os.path.exists(path):
        return None
    with open(path, "r") as f:
        return f.read()

def run_test(description, cmdline, expected_rc=0, expected_out=None,
             expected_err=None, spec="w1 1", inp="alpha beta\n"):
    """Run a single test case.
       cmdline:      the arguments to specs, as a single string.  The tokens
                     @spec@, @in@ and @out@ are replaced by the names of the
                     spec file, the input file and the output file.
       expected_rc:  the expected return code
       expected_out: if not None, the expected content of the output file
       expected_err: if not None, a substring expected in the standard error
       spec:         the content written to the spec file
       inp:          the content written to the input file
    """
    global case_counter, fail_counter, tests_to_run
    case_counter += 1
    if tests_to_run is not None and str(case_counter) not in tests_to_run:
        return

    with open(specfile, "w") as f:
        f.write(spec)
    with open(inpfile, "w") as f:
        f.write(inp)
    if os.path.exists(outfile):
        os.remove(outfile)

    args = [arg.replace("@spec@", specfile).replace("@in@", inpfile)
               .replace("@out@", outfile) for arg in shlex.split(cmdline)]
    with open(errfile, "w") as ef:
        result = subprocess.run([specs_exe] + args, stderr=ef)
    rc = result.returncode

    actual_out = read_file(outfile) or ""
    actual_err = read_file(errfile) or ""

    passed = True
    reason = ""

    if rc != expected_rc:
        passed = False
        reason = "expected rc={} but got rc={}; stderr: {}".format(
            expected_rc, rc, actual_err.strip())
    elif expected_out is not None and actual_out != expected_out:
        passed = False
        reason = "output mismatch.\n  Expected: {}\n  Actual:   {}".format(
            repr(expected_out), repr(actual_out))
    elif expected_err is not None and expected_err not in actual_err:
        passed = False
        reason = "stderr mismatch.\n  Expected to contain: {}\n  Actual:   {}".format(
            repr(expected_err), repr(actual_err.strip()))

    if passed:
        sys.stdout.write("Test #{:03d}: OK    - {}\n".format(case_counter, description))
    else:
        sys.stdout.write("Test #{:03d}: FAIL  - {}\n  {}\n".format(
            case_counter, description, reason))
        fail_counter += 1


def cleanup():
    for f in [specfile, inpfile, outfile, errfile]:
        if os.path.exists(f):
            os.remove(f)

# =====================================================================
# Parse command-line arguments
# =====================================================================
parser = argparse.ArgumentParser(description="Command-line argument tests for specs")
parser.add_argument("--only", dest="only", action="store", default="",
                    help="Comma-separated list of test numbers to run")
args = parser.parse_args()
if args.only != "":
    tests_to_run = args.only.split(",")


# =====================================================================
# A spec file and command-line spec units are mutually exclusive
# =====================================================================

# Test: a spec file on its own works
run_test(
    "Spec file alone",
    "-f @spec@ -i @in@ -o @out@",
    expected_out="alpha\n"
)

# Test: command-line spec units on their own work
run_test(
    "Command-line spec units alone",
    "-i @in@ -o @out@ w1 1",
    expected_out="alpha\n"
)

# Test: a spec file plus trailing spec units is rejected
run_test(
    "Spec file combined with spec units is rejected",
    "-f @spec@ -i @in@ -o @out@ w2 nw",
    expected_rc=252,
    expected_err="cannot be combined with spec units"
)

# Test: the rejection names the offending argument
run_test(
    "Rejection message names the first extraneous argument",
    "-f @spec@ -i @in@ -o @out@ w2 nw",
    expected_rc=252,
    expected_err="<w2>"
)

# Test: even a single extraneous argument is rejected
run_test(
    "Spec file combined with a single spec unit is rejected",
    "-f @spec@ -i @in@ -o @out@ 1-*",
    expected_rc=252,
    expected_err="cannot be combined with spec units"
)

# Test: the switch order does not matter - spec units after the spec file
#       are rejected even when the spec file is named last among the switches
run_test(
    "Rejected regardless of switch order",
    "-i @in@ -o @out@ -f @spec@ w2 nw",
    expected_rc=252,
    expected_err="cannot be combined with spec units"
)

# Test: --info still works when a spec file is given, since it exits early
run_test(
    "A spec file does not interfere with --info",
    "--info -f @spec@",
    expected_err="specs invoked as"
)


# =====================================================================
# Done
# =====================================================================
cleanup()

sys.stdout.write("\n")
if fail_counter > 0:
    sys.stdout.write("*** {} out of {} tests FAILED.\n".format(fail_counter, case_counter))
    sys.exit(1)
else:
    sys.stdout.write("*** All {} tests passed.\n\n".format(case_counter))
