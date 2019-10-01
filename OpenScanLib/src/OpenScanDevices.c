#include "OpenScanLibPrivate.h"

#include "OpenScanDeviceModules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Until we update the API to have proper array memory management, we fill this
// static array once and never modify it again.
static OScDev_PtrArray *g_deviceInstances; // Elements: struct OScInternal_Device*


static void EnumerateDevicesForImpl(const char *moduleName, OScDev_DeviceImpl *impl)
{
	OSc_Error err;
	OScDev_PtrArray *devices = NULL;
	if (OSc_CHECK_ERROR(err, impl->EnumerateInstances(&devices))) {
		char msg[OSc_MAX_STR_LEN + 1];
		const char *model = NULL;
		impl->GetModelName(&model);
		if (!model) {
			model = "(unknown)";
		}

		// The device module should not create the array if it encounters an
		// error. But that is a likely programming error, so catch it.
		if (devices) {
			snprintf(msg, sizeof(msg), "Device enumeration created an array "
				"despite returning an error (this is a memory leak): %s", model);
			// We do not attempt to free the array, because the elements may
			// or may not be valid.
		}

		snprintf(msg, sizeof(msg), "Cannot enumerate devices: %s", model);
		OSc_Log_Warning(NULL, msg);
		return;
	}

	if (!devices) {
		return; // No devices
	}

	for (size_t i = 0; i < devices->size; ++i) {
		struct OScInternal_Device *device = devices->ptr[i];
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
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, OSc_DeviceModule_Get_Count(&nModules)))
		return err;
	char **moduleNames = malloc(sizeof(void *) * nModules);
	if (OSc_CHECK_ERROR(err, OSc_DeviceModule_Get_Names(moduleNames, &nModules)))
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
				(OScDev_DeviceImpl *)(deviceImpls->ptr[i]));
		}
		OSc_PtrArray_Destroy(deviceImpls);
	}

	free(moduleNames);
	return OSc_Error_OK;
}


OSc_Error OSc_GetAllDevices(OSc_Device ***devices, size_t *count)
{
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, EnumerateDevices()))
		return err;

	*devices = (struct OScInternal_Device **)g_deviceInstances->ptr;
	*count = g_deviceInstances->size;
	return OSc_Error_OK;
}


OSc_Error OSc_GetNumberOfAvailableDevices(size_t *count)
{
	EnumerateDevices();
	*count = g_deviceInstances->size;

	return OSc_Error_OK;
}
