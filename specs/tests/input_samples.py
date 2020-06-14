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

ls_out_hdr = \
"""
total 80
-rw-r--r--   1 synp  staff   1941 Jun 12 20:58 Makefile
-rw-r--r--   1 synp  staff   6291 Apr  7 14:25 Makefile.cached_depends
-rw-r--r--   1 synp  staff   6347 Apr  7 14:25 Makefile.cached_depends_vs
drwxr-xr-x   7 synp  staff    224 Jun 12 22:28 cli
-rwxr-xr-x   1 synp  staff    170 Oct 22  2019 mkcache.sh
drwxr-xr-x  20 synp  staff    640 Jun 12 22:28 processing
-rw-r--r--   1 synp  staff  15146 Apr  7 14:25 setup.py
drwxr-xr-x  10 synp  staff    320 Jun 12 22:28 specitems
drwxr-xr-x  18 synp  staff    576 Jun 12 22:29 test
drwxr-xr-x  32 synp  staff   1024 Jun 12 22:29 utils
"""

ls_out_inodes = \
"""
8630081633 Makefile
8630080989 Makefile.cached_depends
8630080990 Makefile.cached_depends_vs
8631159018 clang_on_centos.txt
8631159021 clang_on_mac.txt
8630953247 clc.txt
8618332863 cli
8630994304 clm.txt
8630080994 mkcache.sh
8630080991 patch.txt
8618569561 processing
8630081005 setup.py
8618553163 specitems
8630997293 test
8619309532 utils
8630997296 xxx
"""

ls_out_inodes_mismatched = \
"""
8630081633 Makefile
8630080989 Makefile.cached_depends
8630080990 Makefile.cached_depends_vs
8630953247 clc.txt
8618332863 cli
8630994304 clm.txt
8630080994 mkcache.sh
8630080991 patch.txt
8618569561 processing
8630081005 setup.py
8618553163 specitems
8630997293 test
8619309532 utils
8630997296 xxx
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

employees = \
'''
Payroll,Eddy,Eck
Payroll,Janel,Polen
Finance,Leonard,Cockett
Finance,Dorie,Lugo
Finance,Wiley,Cheung
Finance,Carmelo,Reitz
Finance,Donetta,Rybak
R&D,Jamaal,Mcgillis
R&D,Jonna,Scheffer
R&D,Shawnna,Driskell
R&D,Maybell,Ditmore
R&D,Ami,Fentress
R&D,Randee,Tarkington
R&D,Jerica,Jimenez
Sales,Kristopher,Lind
Sales,Margret,Picone
Sales,Damien,Daniel
Support,Deann,Rushton
Support,Spencer,Marse
Support,Devora,Fortier
'''

gitlog = \
'''
commit df3438ed9e95c2aa37a429ab07f0956164ec4229
Author: synp71 <yoav.nir@gmail.com>
Date:   Sun Jan 20 21:40:41 2019 +0200

    Add NEWS section to Readme.md

commit e6d7f9ac591379d653a5685f9d75deccc1792545
Author: synp71 <yoav.nir@gmail.com>
Date:   Sun Jan 20 21:09:47 2019 +0200

    Issue #33: Some more docs improvement
    
    Also fixed the stats to conform to current timestamp format.

commit 241002cf5a66737bbfd29888244a0a463cd9bcae
Author: synp71 <yoav.nir@gmail.com>
Date:   Thu Jan 17 23:45:21 2019 +0200

    Issue #33: fix formatting

commit 9efb13277c561a3a28195d469420031add60946e
Author: synp71 <yoav.nir@gmail.com>
Date:   Thu Jan 17 23:38:01 2019 +0200

    Issue #33 basic specification and CLI switches
'''

httplog = \
'''
test8:mmind.wariat.org - - [04/Jul/1995:08:12:26 -0400] "GET /shuttle/countdown/video/livevideo.gif HTTP/1.0" 304 0
test8:bruosh01.brussels.hp.com - - [04/Jul/1995:08:12:26 -0400] "GET /shuttle/missions/sts-71/mission-sts-71.html HTTP/1.0" 200 12418
test8:beastie-ppp1.knoware.nl - - [04/Jul/1995:08:12:26 -0400] "GET /shuttle/missions/sts-71/images/KSC-95EC-0423.txt HTTP/1.0" 200 1224
test8:piweba3y.prodigy.com - - [04/Jul/1995:08:12:28 -0400] "GET /shuttle/countdown/liftoff.html HTTP/1.0" 200 4535
test8:sullivan.connix.com - - [04/Jul/1995:08:12:28 -0400] "GET /shuttle/missions/sts-71/images/index71.gif HTTP/1.0" 200 57344
test8:bruosh01.brussels.hp.com - - [04/Jul/1995:08:12:33 -0400] "GET /shuttle/missions/sts-71/sts-71-patch-small.gif HTTP/1.0" 200 12054
test9:mmind.wariat.org - - [04/Jul/1995:08:12:33 -0400] "GET /shuttle/countdown/liftoff.html HTTP/1.0" 304 0
test9:www-d4.proxy.aol.com - - [04/Jul/1995:08:12:34 -0400] "GET /shuttle/missions/sts-71/sts-71-day-01-highlights.html HTTP/1.0" 200 2722
test9:mmind.wariat.org - - [04/Jul/1995:08:12:35 -0400] "GET /shuttle/countdown/video/livevideo.gif HTTP/1.0" 304 0
test9:eepc50.ee.surrey.ac.uk - - [04/Jul/1995:08:12:35 -0400] "GET /shuttle/countdown/video/livevideo.jpeg HTTP/1.0" 200 50437
test10:piweba3y.prodigy.com - - [04/Jul/1995:08:12:37 -0400] "GET /shuttle/countdown/video/livevideo.gif HTTP/1.0" 200 61490
test10:crocus-fddi.csv.warwick.ac.uk - - [04/Jul/1995:08:12:39 -0400] "GET /shuttle/missions/sts-71/mission-sts-71.html HTTP/1.0" 200 12418
test10:crocus-fddi.csv.warwick.ac.uk - - [04/Jul/1995:08:12:41 -0400] "GET /shuttle/missions/sts-71/sts-71-patch-small.gif HTTP/1.0" 200 12054
'''

matrix_with_marks = \
'''
mark 6    7    8    9
4    mark 6    7    8
3    4    mark 6    7 
2    3    4    mark 6
1    2    3    4    5
'''

