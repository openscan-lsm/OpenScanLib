OpenScan C Library
==================

This is the main software module of OpenScan, which provides a C API to control
LSM devices.

The library also exposes a C device programming interface ("DPI") for
implementing modules that control specific devices.


Device Programming Interface
----------------------------

Device control code is packaged into device-specific modules (DLLs on
Windows). When built, these modules have filename extension `.osdev` (instead
of `.dll`, `.so`, etc.).

Device modules are built by linking with the
[OpenScanDeviceLib](OpenScanDeviceLib/README.md) library.


Application Programming Interface
---------------------------------

Applications using OpenScan (such as the µManager OpenScan device adapter)
should link to **OpenScanLib** and include `OpenScanLib.h`. OpenScanLib
discovers and loads device modules, and presents a device-independent
programming interface to the application.


Building
--------

Only Windows is currently supported.

OpenScanLib has one external dependency:
[RichErrors](https://github.com/marktsuchida/RichErrors). Clone RichErrors
(under the same parent directory as OpenScanLib) and build using the following
commands (to place the headers and libraries in the correct location). These
commands must be run in the **x64 Native Tools Command Prompt for VS 2019**,
which can be launched by searching in the Start Menu.

```
cd /d C:\path\to\RichErrors
meson setup --buildtype=debug build-Debug
ninja -C build-Debug

meson setup --buildtype=release build-Release
ninja -C build-Release
```
(This builds, but does not install, the static libraries for Debug and Release
configuration.)

After the above steps, all projects in `OpenScanLib.sln` should build in Visual
Studio 2019.


Code of Conduct
---------------

[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.0-4baaaa.svg)](https://github.com/openscan-lsm/OpenScan/blob/main/CODE_OF_CONDUCT.md)

