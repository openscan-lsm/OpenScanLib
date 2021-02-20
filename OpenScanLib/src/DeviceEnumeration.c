#include "OpenScanLibPrivate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Until we update the API to have proper array memory management, we fill this
// static array once and never modify it again.
static OScInternal_PtrArray *g_deviceInstances; // Elements: struct OScInternal_Device*


static void EnumerateDevicesForImpl(const char *moduleName, OScDev_DeviceImpl *impl)
{
	OScDev_Error errCode;
	OScInternal_PtrArray *devices = NULL;
	errCode = impl->EnumerateInstances(&devices);
	if (errCode) {
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
		OScInternal_LogWarning(NULL, msg);
		return;
	}

	if (!devices) {
		return; // No devices
	}

	for (size_t i = 0; i < OScInternal_PtrArray_Size(devices); ++i) {
		struct OScInternal_Device *device = OScInternal_PtrArray_At(devices, i);
		if (!device) {
			continue;
		}
		OScInternal_PtrArray_Append(g_deviceInstances, device);
	}
}


static OSc_RichError *EnumerateDevices(void)
{
	// For now, enumerate once and for all
	if (g_deviceInstances)
		return OSc_Error_OK;

	size_t nModules;
	OSc_RichError *err;
	if (OSc_CHECK_ERROR(err, OScInternal_DeviceModule_GetCount(&nModules)))
		return err;
	char **moduleNames = malloc(sizeof(void *) * nModules);
	if (OSc_CHECK_ERROR(err, OScInternal_DeviceModule_GetNames(moduleNames, &nModules)))
	{
		free(moduleNames);
		return err;
	}

	g_deviceInstances = OScInternal_PtrArray_Create();
	if (!g_deviceInstances) {
		return OScInternal_Error_Create(OScInternal_Error_OScDomain(), OSc_Error_Unknown, "Error unknown."); // Out of memory
	}

	for (size_t i = 0; i < nModules; ++i)
	{
		const char *moduleName = moduleNames[i];

		OScInternal_PtrArray *deviceImpls = NULL;
		err = OScInternal_DeviceModule_GetDeviceImpls(moduleName, &deviceImpls);
		if (err) {
			char msg[OSc_MAX_STR_LEN + 1] = "Cannot get device implementations from module: ";
			strncat(msg, moduleName, sizeof(msg) - 1);
			msg[sizeof(msg) - 1] = '\0';
			OScInternal_LogWarning(NULL, msg);

			continue;
		}

		for (size_t i = 0; i < OScInternal_PtrArray_Size(deviceImpls); ++i) {
			EnumerateDevicesForImpl(moduleName,
				(OScDev_DeviceImpl *)OScInternal_PtrArray_At(deviceImpls, i));
		}
		OScInternal_PtrArray_Destroy(deviceImpls);
	}

	free(moduleNames);
	return OSc_Error_OK;
}


OSc_RichError *OSc_GetAllDevices(OSc_Device ***devices, size_t *count)
{
	OSc_RichError *err;
	if (OSc_CHECK_ERROR(err, EnumerateDevices()))
		return err;

	*devices = (struct OScInternal_Device **)OScInternal_PtrArray_Data(g_deviceInstances);
	*count = OScInternal_PtrArray_Size(g_deviceInstances);
	return OSc_Error_OK;
}


OSc_RichError *OSc_GetNumberOfAvailableDevices(size_t *count)
{
	EnumerateDevices();
	*count = OScInternal_PtrArray_Size(g_deviceInstances);

	return OSc_Error_OK;
}
