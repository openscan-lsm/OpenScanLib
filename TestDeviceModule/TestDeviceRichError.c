#include "OpenScanDeviceLib.h"
#include "DeviceModules.h"

#include <stdlib.h>
#include <string.h>



static OScDev_Error TestGetModelName(const char **name)
{
	*name = "TestDevice2";
	return OScDev_OK;
}


static OScDev_Error TestEnumerateInstances(OScDev_PtrArray **devices)
{
	*devices = OScDev_PtrArray_Create();

	OScDev_Error errCode;
	OScDev_Device *device0 = NULL;
	errCode = OScDev_Device_Create(&device0, &g_TestDeviceImpl2, NULL);
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
	//strncpy(name, "TestDevice2", OScDev_MAX_STR_LEN);
	//return OScDev_OK;
	//return 6;
	OScDev_RichError *err = OScDev_Error_Create("try");
	return OScDev_Error_ReturnAsCode(err);
}


static OScDev_Error TestOpen(OScDev_Device *device)
{
	//return OScDev_OK;
	OScDev_RichError* err;
	err = OScDev_Error_Create("try");
	OScDev_RichError* err2;
	err2 = OScDev_Error_Wrap(err, "try2");
	return OScDev_Error_ReturnAsCode(err2);
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


OScDev_DeviceImpl g_TestDeviceImpl2 = {
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



