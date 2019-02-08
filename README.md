# specs2016
A re-writing of the specs pipeline stage from CMS, only changed quite a bit

"specs" is a command line utility for parsing and re-arranging text
input. It allows re-alignment of fields, some format conversion, and
re-formatting multiple lines into single lines or vice versa. Input
comes from standard input, and output flows to standard output.

This version is liberally based on the [**CMS Pipelines User's Guide and Reference**](https://publib.boulder.ibm.com/epubs/pdf/hcsj0c30.pdf), especially chapters 16, 24, and 20.

News
====
01-Mar-2019: Version 0.3 Alpha is here
--------------------------------------
What's new:
* The `UNREAD` and `REDO` keywords
* Control breaks, both as the `BREAK` keyword and the `break()` pseudo-function.
* Updated [documentation](specs/docs/TOC.md).

31-Jan-2019: Version 0.2 is out. New stuff:
-------------------------------------------
* **if** and **while** structures.
* **Run-In** and **Run-Out** cycles.
* New functions: `iterno`, `recno`, `conf`, `first`, `eof`, string functions, input line functions and the token `@@`.
* Time/Date improvements: TOD is not expressed in seconds; `tf2d` and `d2tf` conversions.
* ALU debugging for usability.
* Bug fixes.
* [Documentation](https://github.com/yoavnir/specs2016/blob/stable/specs/docs/TOC.md) here on GitHub. 

Sources
=======
To download your copy of *specs*, you can get it from [github](https://github.com/yoavnir/specs2016) in either of two ways:
1. Using git: `git clone https://github.com/yoavnir/specs2016.git`
2. Using http: `wget https://github.com/yoavnir/specs2016/archive/dev.zip`

Building
========
If you have downloaded a git repository, first make sure to check out a stable tag such as v0.2:
```
git checkout v0.2
```
You can also choose to checkout alpha or beta tags, but they will obviously be less stable.

A simple way to get the latest stable release is to check out the `stable` branch and rebase to its tip:
```
git checkout stable
```

After that, _cd_ to the specs/src directory, and run the following two commands:
* `make some`
* `sudo make install`

Contributing
============
Anyone can contribute. So far, I have written all of the code, but if you want to help, I'll be very happy. Feel free to:
* Submit bug reports or feature requests at the [Issue Tracker](https://github.com/yoavnir/specs2016/issues).
* Help solve some existing issue.
* Submit pull requests

Documentation
=============
The documentation for *specs2016* exists in two places:
* In the *manpage* installed with the utility
* In the [docs](specs/docs/TOC.md) directory.

License
=======
*specs2016* is licensed under the [MIT License](https://github.com/yoavnir/specs2016/blob/dev/LICENSE).
