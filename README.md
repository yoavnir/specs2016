# specs2016
A re-writing of the specs pipeline stage from CMS, only changed quite a bit

"specs" is a command line utility for parsing and re-arranging text
input. It allows re-alignment of fields, some format conversion, and
re-formatting multiple lines into single lines or vice versa. Input
comes from standard input, and output flows to standard output.

This version is liberally based on the [**CMS Pipelines User's Guide and Reference**](https://publib.boulder.ibm.com/epubs/pdf/hcsj0c30.pdf), especially chapters 16, 24, and 20.

News
====
11-Sep-2026: Version 1.0.0 is here

What's new:
 * Various TBD improvements
***
1-May-2026: Version 0.9.9 is here

What's new:
 * MSI package & standalone executable for Windows
 * .pkg package for Mac OS
 * RPM for Linux
 * .deb package for Ubuntu/Debian
 * Homebrew formula
 * Improved guessing of Python version
 * New spec units: `SPLITW` and `SPLITF` for splitting input records by words or fields into multiple output records. These new spec units support optional custom separators, `OF` clauses (with the same semantics as SUBSTRING), and range output placement (e.g. `splitw 1-10`).
 * A more exact `exact()` function

*Note:* Installing from package does not include Python support on Windows.

*Note:* On Linux, the `specs` binary is bigger when installed from package, as it is statically linked with libstdc++.

Sources
=======
To download your copy of *specs*, you can get it from [github](https://github.com/yoavnir/specs2016) in either of two ways:
1. Using git: `git clone https://github.com/yoavnir/specs2016.git`
2. Using http: `wget https://github.com/yoavnir/specs2016/archive/dev.zip`

Building
========
If you have downloaded a git repository, first make sure to check out a stable tag such as v0.9.9:
```
git checkout v0.9.9
```
A good way to get the latest stable release is to check out the `stable` branch and rebase to its tip:
```
git checkout stable
git rebase
```

After that, _cd_ to the specs/src directory, and run the following three commands:
* `python setup.py` - use `python3` or `python3.x` if your default Python version is 2.7
* `make some`
* `sudo make install`

*Note:* Windows does not need `sudo`. 

*Note:* Only Python 3 is supported at this point. To enable Python support, you need to have the `python3-devel` package that matches your python version installed.

*Note:* On some Mac machines, `sudo make install` will cause a warning about being the wrong user.

Known Issues
============
* Regular expression grammars other than the default `ECMAScript` don't work except on Mac OS.
* On Windows with Python support the appropriate dll (like `python38.dll`) must be in the path.

Contributing
============
Anyone can contribute. So far, I have written most of the code, but if you want to help, I'll be very happy. Feel free to:
* Submit bug reports or feature requests at the [Issue Tracker](https://github.com/yoavnir/specs2016/issues).
* Help solve some existing issue.
* Submit pull requests
* Even if you use only Windows or only Linux, make sure to update both the `setup.py` file and the relevant `vcxproj` file or files.

New Versions
============
When starting a new version:
* Update the README file
* Update the manpage
* Update specs/Directory.Build.props

Contributors
============
* Yoav Nir ([yoavnir](https://github.com/yoavnir))
* Jean-Baptiste Jouband ([Gawesomer](https://github.com/Gawesomer))

Documentation
=============
The documentation for *specs2016* exists in two places:
* In the *manpage* installed with the utility on Linux and Mac OS.
* In the [docs](specs/docs/TOC.md) directory.

License
=======
*specs2016* is licensed under the [MIT License](https://github.com/yoavnir/specs2016/blob/dev/LICENSE).
