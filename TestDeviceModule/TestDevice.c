#include "OpenScanDeviceLib.h"

#include <stdlib.h>
#include <string.h>


static OScDev_Device **g_devices;
static size_t g_deviceCount;


static struct OScDev_DeviceImpl g_TestDeviceImpl;


static OScDev_Error TestGetModelName(const char **name)
{
	*name = "TestDevice";
	return OScDev_OK;
}


static OScDev_Error TestGetInstances(OScDev_Device ***devices, size_t *count)
{
	OScDev_Error err;

	if (!g_devices)
	{
		g_devices = malloc(sizeof(void *) * 1);
		if (OScDev_CHECK(err, OScDev_Device_Create(&g_devices[0], &g_TestDeviceImpl, NULL)))
		{
			free(g_devices);
			g_devices = NULL;
			return err;
		}
		g_deviceCount = 1;
	}

	*devices = g_devices;
	*count = g_deviceCount;
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


static struct OScDev_DeviceImpl g_TestDeviceImpl = {
	.GetModelName = TestGetModelName,
	.GetInstances = TestGetInstances,
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
	static struct OScDev_DeviceImpl *arr[] = {
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
