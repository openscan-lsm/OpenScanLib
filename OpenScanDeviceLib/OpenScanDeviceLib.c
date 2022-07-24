#include "OpenScanDeviceLibPrivate.h"


struct OScDevInternal_Interface *OScDevInternal_FunctionTable;


uint32_t OScDevInternal_ENTRY_POINT(struct OScDevInternal_Interface ***devif,
	OScDev_ModuleImpl **impl)
{
	{
		// In lieu of a static assert, allow the compiler to produce a warning if
		// signature doesn't match.
		OScDevInternal_EntryPointPtr entryPoint = OScDevInternal_ENTRY_POINT;
		(void)entryPoint;
	}

	*devif = &OScDevInternal_FunctionTable;
	*impl = &OScDevInternal_TheModuleImpl;

	return OScDevInternal_ABI_VERSION;
}
