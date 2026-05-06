Building specs
==============

Sources
-------
To download your copy of *specs*, you can get it from [github](https://github.com/yoavnir/specs2016) in either of two ways:
1. Using git: `git clone https://github.com/yoavnir/specs2016.git`
2. Using http: `wget https://github.com/yoavnir/specs2016/archive/dev.zip`

Prerequisites
-------------
* A C++17-compatible compiler (GCC, Clang, or MSVC)
* Python 3 (optional, for Python integration support)
  * On Linux: the `python3-devel` (or `python3-dev`) package that matches your Python version
  * On Mac OS: the Xcode command-line tools (which include Python headers)
  * On Windows: a standard Python 3 installation includes the required headers and libraries

Checking out a stable version
-----------------------------
If you have downloaded a git repository, first make sure to check out a stable tag such as v0.9.9:
```
git checkout v0.9.9
```
A good way to get the latest stable release is to check out the `stable` branch and rebase to its tip:
```
git checkout stable
git rebase
```

Building on Linux and Mac OS (make)
------------------------------------
Change to the `specs/src` directory, and run the following commands:
1. `python setup.py` -- use `python3` or `python3.x` if your default Python version is 2.7
2. `make some`
3. `sudo make install`

The `setup.py` script auto-detects your compiler, Python installation, and platform capabilities. It generates a `Makefile` tailored to your environment.

### Python support
Python support is detected automatically by `setup.py`. To explicitly control it:
* `python setup.py --python python3.11` -- use a specific Python version
* `python setup.py --python no` -- disable Python support entirely

Only Python 3 is supported. To enable Python support, you need the `python3-devel` package (or equivalent) that matches your Python version installed.

### Notes
* On some Mac machines, `sudo make install` will cause a warning about being the wrong user.
* You can pass `-v DEBUG` to `setup.py` to build a debug version.
* You can pass `--static` to `setup.py` to statically link libstdc++ (useful for portable binaries).

Building on Windows with MSBuild
---------------------------------
Start from the repository root directory (do **not** change to `specs/src`).

### Without Python support (default)
```
msbuild specs\specs.sln /p:Configuration=Release /p:Platform=x64
```

### With Python support
To build with Python support, add `/p:EnablePython=true` to the command line. Python 3 and its development files must be installed on the build machine:
```
msbuild specs\specs.sln /p:Configuration=Release /p:Platform=x64 /p:EnablePython=true
```

Python is auto-detected from the system PATH. If Python is not in your PATH or you want to use a specific installation, provide the installation directory explicitly:
```
msbuild specs\specs.sln /p:Configuration=Release /p:Platform=x64 /p:EnablePython=true /p:PythonDir=C:\Python312
```

You may also override the detected version numbers if needed:
```
msbuild specs\specs.sln /p:Configuration=Release /p:Platform=x64 /p:EnablePython=true /p:PythonDir=C:\Python312 /p:PythonVerNoDot=312 /p:PythonFullVer=3.12.0
```

### After building
Copy the resulting `specs.exe` from the `specs\bin\Release\` directory to a location in your PATH.

### Notes
* With Python support enabled, the appropriate Python DLL (e.g., `python312.dll`) must be in the PATH at runtime.
* To build the Debug configuration, replace `Release` with `Debug` in the commands above.

Building on Windows with make
-----------------------------
As an alternative to MSBuild, you can use `make` on Windows. Change to the `specs/src` directory and run:
1. `python setup.py -c VS`
2. `make some`

This approach uses the Visual Studio `cl.exe` compiler via `make` and supports the same `--python` flag as on other platforms.

Known Issues
------------
* Regular expression grammars other than the default `ECMAScript` don't work except on Mac OS.
* On Windows with Python support, the appropriate DLL (like `python312.dll`) must be in the PATH.

*Note:* Although Windows for ARM64 is not officially supported, that platform will run the x64 version just fine.
