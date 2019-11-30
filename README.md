# specs2016
A re-writing of the specs pipeline stage from CMS, only changed quite a bit

"specs" is a command line utility for parsing and re-arranging text
input. It allows re-alignment of fields, some format conversion, and
re-formatting multiple lines into single lines or vice versa. Input
comes from standard input, and output flows to standard output.

This version is liberally based on the [**CMS Pipelines User's Guide and Reference**](https://publib.boulder.ibm.com/epubs/pdf/hcsj0c30.pdf), especially chapters 16, 24, and 20.

News
====
30-Nov-2019: Version 0.5 GA is here
-------------------------------------
What's new:
* Functions with elided arguments and variable number of arguments
* Some functions reformed because of this.
* Specification path
* Support record formats and locales
* New functions: fmt for formatting numbers. next and rest for formatting records. @cols and @rows.
* Bug fixes

15-Aug-2019: Version 0.4 is here
--------------------------------
What's new:
* New functions: statistical, trigonometric, random
* All the REXX-based functions from CMS Pipelines specs.
* Composed output position ([issue](https://github.com/yoavnir/specs2016/issues/47))
* Multiple input and output streams
* The `ASSERT`, `ABEND` ([issue](https://github.com/yoavnir/specs2016/issues/78)), and `NOWRITE` ([issue](https://github.com/yoavnir/specs2016/issues/80)) keywords.
* Updated [documentation](specs/docs/TOC.md).

**Known Bug** Some unit tests related to the C2F() function fail on Windows.


Sources
=======
To download your copy of *specs*, you can get it from [github](https://github.com/yoavnir/specs2016) in either of two ways:
1. Using git: `git clone https://github.com/yoavnir/specs2016.git`
2. Using http: `wget https://github.com/yoavnir/specs2016/archive/dev.zip`

Building
========
If you have downloaded a git repository, first make sure to check out a stable tag such as v0.4:
```
git checkout v0.4
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

Contributing
============
Anyone can contribute. So far, I have written all of the code, but if you want to help, I'll be very happy. Feel free to:
* Submit bug reports or feature requests at the [Issue Tracker](https://github.com/yoavnir/specs2016/issues).
* Help solve some existing issue.
* Submit pull requests

Documentation
=============
The documentation for *specs2016* exists in two places:
* In the *manpage* installed with the utility on Linux and Mac OS.
* In the [docs](specs/docs/TOC.md) directory.

License
=======
*specs2016* is licensed under the [MIT License](https://github.com/yoavnir/specs2016/blob/dev/LICENSE).
