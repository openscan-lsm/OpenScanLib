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

	OScDev_Error err;
	OScDev_Device *device0 = NULL;
	if (OScDev_CHECK(err, OScDev_Device_Create(&device0, &g_TestDeviceImpl, NULL))) {
		OScDev_PtrArray_Destroy(*devices);
		*devices = NULL;
		return err;
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


OScDev_Error TestGetDeviceImpls(const OScDev_PtrArray **deviceImpls)
{
	static OScDev_DeviceImpl *arr[] = {
		&g_TestDeviceImpl
	};

	OScDev_STATIC_PTR_ARRAY(impls, arr);

	*deviceImpls = &impls;
	return OScDev_OK;
}


OScDev_MODULE_IMPL =
{
	.displayName = "Test Device Module for OpenScan",
	.GetDeviceImpls = TestGetDeviceImpls,
};
