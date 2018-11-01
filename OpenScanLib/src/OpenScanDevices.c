#include "OpenScanLibPrivate.h"

#include "OpenScanDeviceImpl.h"
#include "OpenScanDeviceModules.h"

#include <string.h>


static OSc_Device **g_devices;
static size_t g_deviceCount;


static OSc_Error EnumerateDevices(void)
{
	// For now, enumerate once and for all
	if (g_devices)
		return OSc_Error_OK;

	size_t nModules;
	OSc_Return_If_Error(OSc_DeviceModule_Get_Count(&nModules));
	char **moduleNames = malloc(sizeof(void *) * nModules);
	OSc_Error err;
	if (OSc_Check_Error(err, OSc_DeviceModule_Get_Names(moduleNames, &nModules)))
	{
		free(moduleNames);
		return err;
	}

	for (size_t i = 0; i < nModules; ++i)
	{
		OSc_Device **devices;
		size_t deviceCount;
		if (OSc_Check_Error(err, OSc_DeviceModule_Get_Devices(moduleNames[i], &devices, &deviceCount)))
		{
			// TODO This leaves a partially populated g_devices
			free(moduleNames);
			return err;
		}

		size_t oldCount = g_deviceCount;
		if (!g_devices)
		{
			g_deviceCount = deviceCount;
			g_devices = malloc(sizeof(void *) * deviceCount);
		}
		else
		{
			g_deviceCount += deviceCount;
			g_devices = realloc(g_devices, sizeof(void *) * g_deviceCount);
		}

		for (size_t j = 0; j < deviceCount; ++j)
			g_devices[oldCount + j] = devices[j];
	}

	free(moduleNames);
	return OSc_Error_OK;
}


OSc_Error OSc_Devices_Get_All(OSc_Device ***devices, size_t *count)
{
	OSc_Return_If_Error(EnumerateDevices());

	*devices = g_devices;
	*count = g_deviceCount;
	return OSc_Error_OK;
}


OSc_Error OSc_Devices_Get_Count(size_t *count)
{
	EnumerateDevices();
	*count = g_deviceCount;
	
	return OSc_Error_OK;
}
