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