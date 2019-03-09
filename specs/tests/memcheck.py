import os

RetCode_SUCCESS = 0
RetCode_COMMAND_FAILED = 1
RetCode_DEF_LOST = 2
RetCode_IND_LOST = 3
RetCode_POSS_LOST = 4
RetCode_GENERIC_ERROR = 10

RetCode_strings = ["SUCCESS", "Command Failed", "Definitely Lost", "Indirectly Lost", "Possibly Lost", "", "", "", "", "", "Generic Error"]

def leak_check(cmd):
    cmd_to_execute = "valgrind --leak-check=full --log-file=valgrind.out {} > cmd.out".format(cmd)
    rc = os.system(cmd_to_execute)
    if rc!=0:
        good_return = (RetCode_COMMAND_FAILED,rc)
    else:
        good_return = (RetCode_SUCCESS,0)

    with open("valgrind.out", "r") as f:
        lines = f.readlines()

    no_definitely_lost = False
    no_indirectly_lost = False
    no_possibly_lost = False

    for line in lines:
        if line.find("in use at exit: 0 bytes") >= 0:
            return good_return
        if line.find("definitely lost: 0 bytes") >=0:
            no_definitely_lost = True
        if line.find("indirectly lost: 0 bytes") >=0:
            no_indirectly_lost = True
        if line.find("possibly lost: 0 bytes") >=0:
            no_possibly_lost = True

    if no_definitely_lost and no_indirectly_lost and no_possibly_lost:
        return good_return

    if not no_definitely_lost:
        return (RetCode_DEF_LOST, 0)

    if not no_indirectly_lost:
        return (RetCode_IND_LOST, 0)

    if not no_possibly_lost:
        return (RetCode_POSS_LOST, 0)


def cleanup():
    os.system("/bin/rm valgrind.out cmd.out")

def cleanup_valgrind():
    os.system("/bin/rm valgrind.out")

def leak_check_specs(spec, inp):
    with open("thespec", "w") as s:
        s.write(spec)
    with open("theinp", "w") as i:
        if inp is None:
            i.write("")
        else:
            i.write(inp)
    cmd = "../exe/specs -f thespec -i theinp -o theout"
    return leak_check(cmd)

