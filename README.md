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

[Doxygen documentation for OpenScanDeviceLib](https://openscan-lsm.github.io/OpenScanLib/OpenScanDeviceLib/)


Application Programming Interface
---------------------------------

Applications using OpenScan (such as the µManager OpenScan device adapter)
should link to **OpenScanLib** and include `OpenScanLib.h`. OpenScanLib
discovers and loads device modules, and presents a device-independent
programming interface to the application.

[Doxygen documentation for OpenScanLib API](https://openscan-lsm.github.io/OpenScanLib/OpenScanLib/)


Building
--------

Only Windows/MSVC is currently supported.

Make sure Python is installed an on `PATH`.

[Meson](https://mesonbuild.com/) is used for build (installed as part of the
following instructions).

In Git Bash:

```sh
git clone https://github.com/openscan-lsm/OpenScanLib.git
cd OpenScanLib
python -m venv venv           # Create virtual environment
echo '*' > venv/.gitignore    # Do not track 'venv'
. venv/Scripts/activate
pip install meson ninja
meson setup --vsenv --buildtype=release builddir
meson compile -C builddir
meson test -C builddir
```

OpenScanLib currently has one external dependency:
[RichErrors](https://github.com/marktsuchida/RichErrors). By default it is
automatically fetched using Meson's wrap facility.

Code of Conduct
---------------

[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.0-4baaaa.svg)](https://github.com/openscan-lsm/OpenScan/blob/main/CODE_OF_CONDUCT.md)

