OpenScanDeviceLib
=================

Library for building OpenScan device modules.

See the Doxygen-generated documentation.


How to generate documentation
-----------------------------

1. Make sure Doxygen is installed.
1. In a terminal, `cd` to the `OpenScanDeviceLib` directory.
1. Run `doxygen doc/Doxyfile`.
1. Open `doc/doxygen/html/index.html` in a browser.


How to link to OpenScanDeviceLib in Visual Studio
-------------------------------------------------

1. Place the OpenScanLib project tree and the device module project tree
under the same parent directory.
1. Create a new Win32 DLL project, choosing "Empty Project".
1. In the properties for the new device module project, set _Target
Extension_ to `.osdev`.
1. Add `../OpenScanLib/OpenScanDeviceLib/include` to
_Additional Include Directories_ (under _C/C++_).
1. Add `../OpenScanLib/$(Platform)/$(Configuration)` to
_Additional Library Directories_ (under _Linker_).
1. Add `OpenScanDeviceLib.lib` to _Additional Dependencies_ (under _Linker_).
1. In the source code, include `OpenScanDeviceLib.h`. This is the only header
needed by device modules (make sure you do **not** use any other headers or
libraries from OpenScanLib).