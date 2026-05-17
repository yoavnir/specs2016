import os, sys, argparse, subprocess, shlex

# Test script for record formats (--recfm and --lrecl flags)

specs_exe = "../exe/specs"

case_counter = 0
fail_counter = 0
tests_to_run = None

def write_file(path, content, binary=False):
    mode = "wb" if binary else "w"
    with open(path, mode) as f:
        f.write(content)

def read_file(path):
    if not os.path.exists(path):
        return None
    with open(path, "r") as f:
        return f.read()

def run_test(description, spec, inp, expected, extra_flags="",
             binary_input=False, expected_rc=0):
    """Run a single test case.
       spec:     the specs specification string
       inp:      the input data (string, or bytes if binary_input=True)
       expected: the expected output string
       extra_flags: additional CLI flags like '--recfm f --lrecl 10'
       binary_input: if True, inp is bytes written in binary mode
       expected_rc: 0 for success, non-zero for expected failure
    """
    global case_counter, fail_counter, tests_to_run
    case_counter += 1
    if tests_to_run is not None and str(case_counter) not in tests_to_run:
        return

    specfile = "recfm_spec"
    inpfile = "recfm_inp"
    outfile = "recfm_out"
    errfile = "recfm_err"

    write_file(specfile, spec)
    write_file(inpfile, inp, binary=binary_input)

    cmd_parts = [specs_exe] + shlex.split(extra_flags) + [
        "-f", specfile, "-i", inpfile, "-o", outfile]
    with open(errfile, "w") as ef:
        result = subprocess.run(cmd_parts, stderr=ef)
    rc = result.returncode

    actual = read_file(outfile)
    if actual is None:
        actual = ""

    passed = True
    reason = ""

    if expected_rc == 0 and rc != 0:
        passed = False
        err_text = read_file(errfile) or ""
        reason = "expected success but got rc={}; stderr: {}".format(rc, err_text.strip())
    elif expected_rc != 0 and rc == 0:
        passed = False
        reason = "expected failure (rc!=0) but got rc=0"
    elif expected_rc == 0 and actual != expected:
        passed = False
        reason = "output mismatch.\n  Expected: {}\n  Actual:   {}".format(
            repr(expected), repr(actual))

    if passed:
        sys.stdout.write("Test #{:03d}: OK    - {}\n".format(case_counter, description))
    else:
        sys.stdout.write("Test #{:03d}: FAIL  - {}\n  {}\n".format(
            case_counter, description, reason))
        fail_counter += 1


def cleanup():
    for f in ["recfm_spec", "recfm_inp", "recfm_out", "recfm_err"]:
        if os.path.exists(f):
            os.remove(f)

# =====================================================================
# Parse command-line arguments
# =====================================================================
parser = argparse.ArgumentParser(description="Record format tests for specs")
parser.add_argument("--only", dest="only", action="store", default="",
                    help="Comma-separated list of test numbers to run")
args = parser.parse_args()
if args.only != "":
    tests_to_run = args.only.split(",")


# =====================================================================
# FIXED record format (--recfm f --lrecl N)
# Records are read as fixed-width blocks with no line delimiters.
# =====================================================================

# Test: basic fixed-length records
# Input is 30 bytes, lrecl=10, so 3 records of 10 chars each
run_test(
    "Fixed format - basic 10-byte records",
    "1-* 1",
    b"AAAAAAAAAA" b"BBBBBBBBBB" b"CCCCCCCCCC",
    "AAAAAAAAAA\nBBBBBBBBBB\nCCCCCCCCCC\n",
    extra_flags="--recfm f --lrecl 10",
    binary_input=True
)

# Test: fixed-length records with meaningful content
run_test(
    "Fixed format - extract columns from fixed records",
    "1-3 1 5-8 nw",
    b"ABC EFGH" b"123 5678" b"xyz wxyz",
    "ABC EFGH\n123 5678\nxyz wxyz\n",
    extra_flags="--recfm f --lrecl 8",
    binary_input=True
)

# Test: partial last record is discarded in fixed format
# 25 bytes with lrecl=10: 2 full records, 5 leftover bytes discarded
run_test(
    "Fixed format - partial last record discarded",
    "1-* 1",
    b"AAAAAAAAAA" b"BBBBBBBBBB" b"CCCCC",
    "AAAAAAAAAA\nBBBBBBBBBB\n",
    extra_flags="--recfm f --lrecl 10",
    binary_input=True
)

# Test: fixed format reads through newline characters (they are data)
# Newlines are just regular bytes in fixed-format mode
# "ABCD\n" -> hex "414243440a", "FGHIJ" -> hex "464748494a"
run_test(
    "Fixed format - newlines treated as data",
    "1-* c2x 1",
    b"ABCD\nFGHIJ",
    "414243440a\n464748494a\n",
    extra_flags="--recfm f --lrecl 5",
    binary_input=True
)

# Test: fixed format with lrecl=1 (single-byte records)
run_test(
    "Fixed format - single byte records",
    "1 1",
    b"ABCDE",
    "A\nB\nC\nD\nE\n",
    extra_flags="--recfm f --lrecl 1",
    binary_input=True
)

# Test: word extraction from fixed records
run_test(
    "Fixed format - word extraction",
    "w1 1 w2 nw",
    b"hello world " b"foo   bar   " b"one   two   ",
    "hello world\nfoo bar\none two\n",
    extra_flags="--recfm f --lrecl 12",
    binary_input=True
)


# =====================================================================
# FIXED-DELIMITED record format (--recfm fd --lrecl N)
# Lines are delimited by newlines, but padded/truncated to lrecl.
# =====================================================================

# Test: lines shorter than lrecl are padded with spaces
run_test(
    "Fixed-delimited - short lines padded",
    "1-* 1",
    "hi\nbye\na\n",
    "hi   \nbye  \na    \n",
    extra_flags="--recfm fd --lrecl 5"
)

# Test: lines longer than lrecl are truncated
run_test(
    "Fixed-delimited - long lines truncated",
    "1-* 1",
    "abcdefghij\nklmnopqrst\n",
    "abcde\nklmno\n",
    extra_flags="--recfm fd --lrecl 5"
)

# Test: lines exactly lrecl pass through unchanged
run_test(
    "Fixed-delimited - exact length unchanged",
    "1-* 1",
    "abcde\nfghij\n",
    "abcde\nfghij\n",
    extra_flags="--recfm fd --lrecl 5"
)

# Test: mixed lengths
run_test(
    "Fixed-delimited - mixed line lengths",
    "1-* 1",
    "ab\nabcdefghij\nabcde\n",
    "ab   \nabcde\nabcde\n",
    extra_flags="--recfm fd --lrecl 5"
)

# Test: column extraction with padding
run_test(
    "Fixed-delimited - column extraction on padded data",
    "4-5 1",
    "AB\nABCDE\nABCDEFGH\n",
    "  \nDE\nDE\n",
    extra_flags="--recfm fd --lrecl 5"
)


# =====================================================================
# DELIMITED record format (default, and with --linedel)
# =====================================================================

# Test: default delimited mode (sanity check)
run_test(
    "Delimited - default newline delimiter",
    "1-* 1",
    "hello\nworld\n",
    "hello\nworld\n"
)

# Test: custom line delimiter
run_test(
    "Delimited - custom delimiter (pipe)",
    "1-* 1",
    "hello|world|test",
    "hello\nworld\ntest\n",
    extra_flags="--linedel |"
)


# =====================================================================
# Error cases
# =====================================================================

# Test: fixed format without lrecl should fail at argument parsing
run_test(
    "Error - fixed format without lrecl",
    "1-* 1",
    "hello\n",
    "",
    extra_flags="--recfm f",
    expected_rc=1
)

# Test: fixed-delimited without lrecl should fail
run_test(
    "Error - fixed-delimited without lrecl",
    "1-* 1",
    "hello\n",
    "",
    extra_flags="--recfm fd",
    expected_rc=1
)


# =====================================================================
# Integration: combining record formats with other features
# =====================================================================

# Test: NUMBER with fixed format
run_test(
    "Fixed format - NUMBER source",
    "number 1 /: / nw 1-* nw",
    b"rec1xxxxxx" b"rec2xxxxxx" b"rec3xxxxxx",
    "         1 :  rec1xxxxxx\n         2 :  rec2xxxxxx\n         3 :  rec3xxxxxx\n",
    extra_flags="--recfm f --lrecl 10",
    binary_input=True
)

# Test: word and field operations with fixed-delimited
run_test(
    "Fixed-delimited - word count on padded lines",
    "print 'wordcount()' 1",
    "a b\na b c d e\n",
    "2\n5\n",
    extra_flags="--force-read-input --recfm fd --lrecl 20"
)

# Test: field separator with fixed-delimited
# "alpha,beta,gamma" truncated to "alpha,beta" -> f1="alpha", f2="beta"
# "x,y" padded to "x,y       " -> f1="x", f2="y       "
run_test(
    "Fixed-delimited - field separator with truncation",
    "fs , f1 1 f2 nw",
    "alpha,beta,gamma\nx,y\n",
    "alpha beta\nx y       \n",
    extra_flags="--recfm fd --lrecl 10"
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

