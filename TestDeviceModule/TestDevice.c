#include "OpenScanDeviceImpl.h"
#include "OpenScanLibPrivate.h"

#include <string.h>


static OSc_Device **g_devices;
static size_t g_deviceCount;


struct OSc_Device_Impl OpenScan_Test_Device_Impl;


static OSc_Error TestGetModelName(const char **name)
{
	*name = "TestDevice";
	return OSc_Error_OK;
}


static OSc_Error TestGetInstances(OSc_Device ***devices, size_t *count)
{
	if (!g_devices)
	{
		g_devices = malloc(sizeof(void *) * 1);
		OSc_Return_If_Error(OSc_Device_Create(&g_devices[0], &OpenScan_Test_Device_Impl, NULL));
		g_deviceCount = 1;
	}

	*devices = g_devices;
	*count = g_deviceCount;
	return OSc_Error_OK;
}


static OSc_Error TestReleaseInstance(OSc_Device *device)
{
	return OSc_Error_OK;
}


static OSc_Error TestGetName(OSc_Device *device, char *name)
{
	strncpy(name, "TestDevice", OSc_MAX_STR_LEN);
	return OSc_Error_OK;
}


static OSc_Error TestOpen(OSc_Device *device)
{
	return OSc_Error_OK;
}


static OSc_Error TestClose(OSc_Device *device)
{
	return OSc_Error_OK;
}


static OSc_Error TestHasScanner(OSc_Device *device, bool *hasScanner)
{
	*hasScanner = false;
	return OSc_Error_OK;
}


static OSc_Error TestHasDetector(OSc_Device *device, bool *hasDetector)
{
	*hasDetector = false;
	return OSc_Error_OK;
}


struct OSc_Device_Impl OpenScan_Test_Device_Impl = {
	.GetModelName = TestGetModelName,
	.GetInstances = TestGetInstances,
	.ReleaseInstance = TestReleaseInstance,
	.GetName = TestGetName,
	.Open = TestOpen,
	.Close = TestClose,
	.HasScanner = TestHasScanner,
	.HasDetector = TestHasDetector,
};


OSc_Error OSc_ENTRY_POINT_FUNC(struct OSc_Device_Impl **impls, size_t *implCount)
{
	if (*implCount < 1)
		return OSc_Error_OK;

	impls[0] = &OpenScan_Test_Device_Impl;
	*implCount = 1;
	return OSc_Error_OK;
}