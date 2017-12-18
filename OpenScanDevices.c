#include "OpenScanLibPrivate.h"
#include "OpenScanDeviceImpl.h"

#include "OScNIFPGADevice.h"
#include "OScNIDAQDevice.h"
#include "BH_SPC150.h"

#include <string.h>

static OSc_Device **g_devices;
static size_t g_deviceCount;

static struct OSc_Device_Impl *implementations[] = {
	&OpenScan_NIFPGA_Device_Impl,
	// &OpenScan_NIDAQ_Device_Impl,
	&BH_TCSCP150_Device_Impl,
};


static void EnumerateDevices()
{
	if (g_devices)
		return;

	g_deviceCount = 0;
	for (int i = 0; implementations[i] != NULL; ++i)
	{
		OSc_Device **implDevices;
		size_t implCount;
		OSc_Error err;
		if (OSc_Check_Error(err, implementations[i]->GetInstances(&implDevices, &implCount)))
		{
			char msg[OSc_MAX_STR_LEN + 1] = "Cannot enumerate devices: ";
			const char *model = NULL;
			implementations[i]->GetModelName(&model);
			strcat(msg, model ? model : "(unknown)");
			OSc_Log_Warning(NULL, msg);
			continue;
		}

		if (implCount == 0)
			continue;

		size_t oldCount = g_deviceCount;
		if (!g_devices)
		{
			++g_deviceCount;
			g_devices = malloc(implCount * sizeof(OSc_Device *));
		}
		else
		{
			g_deviceCount += implCount;
			g_devices = realloc(g_devices, g_deviceCount * sizeof(OSc_Device *));
		}

		for (int j = 0; j < implCount; ++j)
		{
			g_devices[oldCount + j] = implDevices[j];
		}
	}
}


OSc_Error OSc_Devices_Get_All(OSc_Device ***devices, size_t *count)
{
	EnumerateDevices();

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