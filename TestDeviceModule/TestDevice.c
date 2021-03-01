#include "OpenScanDeviceLib.h"

#include <stdlib.h>
#include <string.h>


static OScDev_DeviceImpl g_TestDeviceImpl;


static OScDev_Error TestGetModelName(const char **name)
{
	*name = "TestDevice";
	return OScDev_OK;
}


static OScDev_Error TestEnumerateInstances(OScDev_PtrArray **devices)
{
	*devices = OScDev_PtrArray_Create();

	OScDev_Error errCode;
	OScDev_Device *device0 = NULL;
	errCode = OScDev_Device_Create(&device0, &g_TestDeviceImpl, NULL);
	if (errCode) {
		OScDev_PtrArray_Destroy(*devices);
		*devices = NULL;
		return errCode;
	}
	OScDev_PtrArray_Append(*devices, device0);

	return OScDev_OK;
}


static OScDev_Error TestReleaseInstance(OScDev_Device *device)
{
	return OScDev_OK;
}


static OScDev_Error TestGetName(OScDev_Device *device, char *name)
{
	strncpy(name, "TestDevice", OScDev_MAX_STR_LEN);
	return OScDev_OK;
}


static OScDev_Error TestOpen(OScDev_Device *device)
{
	return OScDev_OK;
}


static OScDev_Error TestClose(OScDev_Device *device)
{
	return OScDev_OK;
}


static OScDev_Error TestHasClock(OScDev_Device *device, bool *hasClock)
{
	*hasClock = false;
	return OScDev_OK;
}


static OScDev_Error TestHasScanner(OScDev_Device *device, bool *hasScanner)
{
	*hasScanner = false;
	return OScDev_OK;
}


static OScDev_Error TestHasDetector(OScDev_Device *device, bool *hasDetector)
{
	*hasDetector = false;
	return OScDev_OK;
}


static OScDev_DeviceImpl g_TestDeviceImpl = {
	.GetModelName = TestGetModelName,
	.EnumerateInstances = TestEnumerateInstances,
	.ReleaseInstance = TestReleaseInstance,
	.GetName = TestGetName,
	.Open = TestOpen,
	.Close = TestClose,
	.HasClock = TestHasClock,
	.HasScanner = TestHasScanner,
	.HasDetector = TestHasDetector,
	// Other required methods omitted.
};


OScDev_Error TestGetDeviceImpls(OScDev_PtrArray **deviceImpls)
{
	*deviceImpls = OScDev_PtrArray_CreateFromNullTerminated(
		(OScDev_DeviceImpl *[]){ &g_TestDeviceImpl, NULL });
	return OScDev_OK;
}


OScDev_MODULE_IMPL =
{
	.displayName = "Test Device Module for OpenScan",
	.GetDeviceImpls = TestGetDeviceImpls,
};
