# specs2016
A re-writing of the specs pipeline stage from CMS, only changed quite a bit

"specs" is a command line utility for parsing and re-arranging text
input. It allows re-alignment of fields, some format conversion, and
re-formatting multiple lines into single lines or vice versa. Input
comes from standard input, and output flows to standard output.

This version is liberally based on the [**CMS Pipelines User's Guide and Reference**](https://publib.boulder.ibm.com/epubs/pdf/hcsj0c30.pdf), especially chapters 16, 24, and 20.

News
====
Version 0.6 Alpha will be here on 31-Jan-2020
---------------------------------------------
Planned features:
* Python functions
* MainOptions - see [issue #101](https://github.com/yoavnir/specs2016/issues/101).

Sources
=======
To download your copy of *specs*, you can get it from [github](https://github.com/yoavnir/specs2016) in either of two ways:
1. Using git: `git clone https://github.com/yoavnir/specs2016.git`
2. Using http: `wget https://github.com/yoavnir/specs2016/archive/dev.zip`

Building
========
If you have downloaded a git repository, first make sure to check out a stable tag such as v0.5:
```
git checkout v0.5
```
You can also choose to checkout alpha or beta tags, but they will obviously be less stable.

A simple way to get the latest stable release is to check out the `stable` branch and rebase to its tip:
```
git checkout stable
git rebase
```

After that, _cd_ to the specs/src directory, and run the following three commands:
* `python setup.py`
* `make some`
* `sudo make install`

Note: Windows does not need `sudo`

Known Issues
============
* On CentOS 7 and other Linux distros using GCC 4.8.5 or earlier some Python unit tests fail. Also, importing Python native functions doesn't work.
* On Windows with Python support the appropriate dll (like `python38.dll`) must be in the path.

Contributing
============
Anyone can contribute. So far, I have written all of the code, but if you want to help, I'll be very happy. Feel free to:
* Submit bug reports or feature requests at the [Issue Tracker](https://github.com/yoavnir/specs2016/issues).
* Help solve some existing issue.
* Submit pull requests

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
