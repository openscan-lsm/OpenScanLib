#include "OpenScanLibPrivate.h"

#include "OpenScanDeviceModules.h"

#include <stdlib.h>
#include <string.h>


// Until we update the API to have proper array memory management, we fill this
// static array once and never modify it again.
static OScDev_PtrArray *g_deviceInstances; // Elements: struct OSc_Device*


static void EnumerateDevicesForImpl(const char *moduleName, struct OScDev_DeviceImpl *impl)
{
	OSc_Error err;
	struct OSc_Device **devices;
	size_t count;
	if (OSc_Check_Error(err, impl->GetInstances(&devices, &count))) {
		char msg[OSc_MAX_STR_LEN + 1] = "Cannot enumerate devics: ";
		const char *model = NULL;
		impl->GetModelName(&model);
		strcat(msg, model ? model : "(unknown)");
		OSc_Log_Warning(NULL, msg);
		return;
	}

	for (size_t i = 0; i < count; ++i) {
		struct OSc_Device *device = devices[i];
		if (!device) {
			continue;
		}
		OSc_PtrArray_Append(g_deviceInstances, device);
	}
}


static OSc_Error EnumerateDevices(void)
{
	// For now, enumerate once and for all
	if (g_deviceInstances)
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

	g_deviceInstances = OSc_PtrArray_Create();
	if (!g_deviceInstances) {
		return OSc_Error_Unknown; // Out of memory
	}

	for (size_t i = 0; i < nModules; ++i)
	{
		const char* moduleName = moduleNames[i];

		const OScDev_PtrArray *deviceImpls = NULL;
		err = OSc_DeviceModule_GetDeviceImpls(moduleName, &deviceImpls);
		if (err) {
			char msg[OSc_MAX_STR_LEN + 1] = "Cannot get device implementations from module: ";
			strncat(msg, moduleName, sizeof(msg) - 1);
			msg[sizeof(msg) - 1] = '\0';
			OSc_Log_Warning(NULL, msg);

			continue;
		}

		for (size_t i = 0; i < deviceImpls->size; ++i) {
			EnumerateDevicesForImpl(moduleName,
				(struct OScDev_DeviceImpl *)(deviceImpls->ptr[i]));
		}
		OSc_PtrArray_Destroy(deviceImpls);
	}

	free(moduleNames);
	return OSc_Error_OK;
}


OSc_Error OSc_Devices_Get_All(OSc_Device ***devices, size_t *count)
{
	OSc_Return_If_Error(EnumerateDevices());

	*devices = (struct OSc_Device **)g_deviceInstances->ptr;
	*count = g_deviceInstances->size;
	return OSc_Error_OK;
}


OSc_Error OSc_Devices_Get_Count(size_t *count)
{
	EnumerateDevices();
	*count = g_deviceInstances->size;

	return OSc_Error_OK;
}
