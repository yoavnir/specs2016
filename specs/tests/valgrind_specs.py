import memcheck,sys

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

# Some inputs that we will use multiple times

ls_out = \
"""
-rw-r--r--   1 synp  staff    1462 Mar  9 10:45 Makefile
-rw-r--r--   1 synp  staff    4530 Mar  4 19:36 Makefile.cached_depends
-rw-r--r--   1 synp  staff    4576 Mar  4 19:36 Makefile.cached_depends_vs
-rw-r--r--   1 synp  staff    4997 Feb 27 21:12 clang_on_centos.txt
-rw-r--r--   1 synp  staff   11931 Feb 27 21:14 clang_on_mac.txt
-rw-r--r--   1 synp  staff    4997 Feb 27 21:25 clc.txt
drwxr-xr-x   7 synp  staff     224 Mar  9 10:45 cli
-rw-r--r--   1 synp  staff  172076 Feb 27 22:18 clm.txt
-rwxr-xr-x   1 synp  staff     170 Mar  4 19:36 mkcache.sh
-rw-r--r--   1 synp  staff     771 Mar  9 10:49 patch.txt
drwxr-xr-x  20 synp  staff     640 Mar  9 10:54 processing
-rw-r--r--   1 synp  staff    7341 Mar  4 19:36 setup.py
drwxr-xr-x  10 synp  staff     320 Mar  9 10:45 specitems
drwxr-xr-x  16 synp  staff     512 Mar  9 10:45 test
drwxr-xr-x  19 synp  staff     608 Mar  9 10:45 utils
-rw-r--r--   1 synp  staff   11931 Feb 27 21:39 xxx
"""

Jabberwocky = \
"""
'Twas brillig, and the slithy toves
      Did gyre and gimble in the wabe:
All mimsy were the borogoves,
      And the mome raths outgrabe.

'Beware the Jabberwock, my son!
      The jaws that bite, the claws that catch!
Beware the Jubjub bird, and shun
      The frumious Bandersnatch!'

He took his vorpal sword in hand;
      Long time the manxome foe he sought--
So rested he by the Tumtum tree
      And stood awhile in thought.

And, as in uffish thought he stood,
      The Jabberwock, with eyes of flame,
Came whiffling through the tulgey wood,
      And burbled as it came!

One, two! One, two! And through and through
      The vorpal blade went snicker-snack!
He left it dead, and with its head
      He went galumphing back.

'And hast thou slain the Jabberwock?
      Come to my arms, my beamish boy!
O frabjous day! Callooh! Callay!'
      He chortled in his joy.

'Twas brillig, and the slithy toves
      Did gyre and gimble in the wabe:
All mimsy were the borogoves,
      And the mome raths outgrabe.
"""

# Now come the test cases

s = "hello 1"
i = None
run_case(s,i,"Literal in first column")

s = "2 1"
i = "hello"
run_case(s,i,"Random char in first column")

s = "w2 5"
i = Jabberwocky
run_case(s,i,"Random word in random column")

s = "w2 ucase 5"
i = Jabberwocky
run_case(s,i,"Simple conversion")

s = "w2 c2x 5"
i = Jabberwocky
run_case(s,i,"Another conversion")

s = "w2 x2ch 5"
i = Jabberwocky
run_case(s,i,"A conversion with errors",memcheck.RetCode_COMMAND_FAILED) # Because the words of Jabberwocky are not hex

s = 'a: w5 . set "#0+=a" eof print "#0" 1'
i = ls_out
run_case(s,i,"Summing up a field in the input")

