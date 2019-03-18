import os

RetCode_SUCCESS = 0
RetCode_COMMAND_FAILED = 1
RetCode_DEF_LOST = 2
RetCode_IND_LOST = 3
RetCode_POSS_LOST = 4
RetCode_GENERIC_ERROR = 10

RetCode_strings = ["SUCCESS", "Command Failed", "Definitely Lost", "Indirectly Lost", "Possibly Lost", "", "", "", "", "", "Generic Error"]

no_valgrind = False
keep_specs_output = False

def leak_check(cmd, test_id="0"):
    global no_valgrind,keep_specs_output
    if keep_specs_output:
        valgfile = "valgrind.out." + test_id
        outfile = "cmd.out." + test_id
    else:
    	valgfile = "valgrind.out"
    	outfile = "cmd.out"
    if no_valgrind:
        cmd_to_execute = "{} > {}".format(cmd,outfile)
    else:
        cmd_to_execute = "valgrind --leak-check=full --log-file={} {} > {}".format(valgfile,cmd,outfile)
    rc = os.system(cmd_to_execute)
    if rc!=0:
        good_return = (RetCode_COMMAND_FAILED,rc)
    else:
        good_return = (RetCode_SUCCESS,0)
        
    if no_valgrind:
        return good_return

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
    os.system("/bin/rm valgrind.out* cmd.out* 2> /dev/null")

def cleanup_valgrind():
    os.system("/bin/rm valgrind.out* 2> /dev/null")

def leak_check_specs(spec, inp, testid, confFile):
    global keep_specs_output
    if keep_specs_output:
    	specfile = "thespec."+str(testid)
    	inpfile = "theinp."+str(testid)
    	outfile = "theout."+str(testid)
    	conffile = "theconf."+str(testid)
    else:
    	specfile = "thespec"
    	inpfile = "theinp"
    	outfile = "theout"
    	conffile = "theconf"
    with open(specfile, "w") as s:
        s.write(spec)
    with open(conffile,"w") as c:
    	c.write(confFile)
    with open(inpfile, "w") as i:
        if inp is None:
            i.write("")
        else:
            i.write(inp)
    cmd = "../exe/specs -c {} -f {} -i {} -o {}".format(conffile, specfile,inpfile,outfile)
    return leak_check(cmd,str(testid))

