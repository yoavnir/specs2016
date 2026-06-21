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
 * Rolling context allows processing previous and future records.
 * Packages for Linux, Mac OS and Windows, bundled with Python 3.12.
 * Python support in `MSBuild` builds.
 * Exactness in Python function arguments and return value.
 * Build information system-defined labels, such as `@build-info` and `@build-url`.
 * Debugging aids for GDB.


Sources
=======
To download your copy of *specs*, you can get it from [github](https://github.com/yoavnir/specs2016) in either of two ways:
1. Using git: `git clone https://github.com/yoavnir/specs2016.git`
2. Using http: `wget https://github.com/yoavnir/specs2016/archive/dev.zip`

Installation from binaries
==========================
The binaries for the latest release can be downloaded from [**the release page**](https://github.com/yoavnir/specs2016/releases/tag/v1.0.0)

**Notes:**
 * On Windows for ARM, you may install the x64 version of Python 3.12.
 * Recent Mac OS versions are very strict on where packages come from.  You may need to issue the following command to get the .pkg file to install: `xattr -dr com.apple.quarantine /path/to/specs-1.0.0.pkg`

Building
========
For detailed build instructions covering Linux, Mac OS, and Windows (both `make` and MSBuild), see [BUILDING.md](BUILDING.md).

**Note on Python versions:** The pre-built binaries are linked against Python 3.12, and bundled with it. If you need to use a different version of Python, you can build `specs` locally from source. When building, you can specify which Python version to use via the `--python` option to `setup.py` (on Linux/macOS) or by setting the appropriate Python version in your Visual Studio environment (on Windows).

Known Issues
============
* Regular expression grammars other than the default `ECMAScript` don't work except on Mac OS.
* The Python-enabled Windows MSI bundles its own Python 3.12 runtime, so no system Python is required. The standalone Python-enabled `.exe` (downloaded on its own, outside the MSI) still needs `python312.dll` on the path (or Python 3.12 installed).

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
- Yoav Nir ([yoavnir](https://github.com/yoavnir))
- Jean-Baptiste Jouband ([Gawesomer](https://github.com/Gawesomer))
- donglrd ([donglrd](https://github.com/donglrd))
- Miriam-R-coder ([Miriam-R-coder](https://github.com/Miriam))

Documentation
=============
The documentation for *specs2016* exists in two places:
* In the *manpage* installed with the utility on Linux and Mac OS.
* In the [docs](specs/docs/TOC.md) directory.

License
=======
* *specs2016* is licensed under the [MIT License](https://github.com/yoavnir/specs2016/blob/dev/LICENSE).
* The *Python 3.12* library bundled with the GitHub-built packages is licensed under the [Python Software Foundation License](https://github.com/yoavnir/specs2016/blob/dev-1.0.0/PYTHON_LICENSE)
