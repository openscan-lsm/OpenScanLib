#include "OpenScanLibPrivate.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


OSc_Error OSc_Device_GetName(OSc_Device *device, const char **name)
{
	if (!strlen(device->name))
		OSc_RETURN_IF_ERROR(device->impl->GetName(device, device->name));

	*name = device->name;
	return OSc_Error_OK;
}


OSc_Error OSc_Device_GetDisplayName(OSc_Device *device, const char **name)
{
	if (!strlen(device->displayName))
	{
		const char *modelName;
		OSc_RETURN_IF_ERROR(device->impl->GetModelName(&modelName));
		const char *deviceName;
		OSc_RETURN_IF_ERROR(OSc_Device_GetName(device, &deviceName));
		snprintf(device->displayName, OSc_MAX_STR_LEN, "%s@%s",
			modelName, deviceName);
	}

	*name = device->displayName;
	return OSc_Error_OK;
}

OSc_Error OSc_Device_Open(OSc_Device *device, OSc_LSM *lsm)
{
	if (device->isOpen)
	{
		if (device->associatedLSM == lsm)
			return OSc_Error_OK;
		return OSc_Error_Device_Already_Open;
	}

	OSc_RETURN_IF_ERROR(device->impl->Open(device));
	device->isOpen = true;

	OSc_Error err;

	bool hasClock;
	if (OSc_CHECK_ERROR(err, OSc_Device_HasClock(device, &hasClock)))
		goto Error;
	if (hasClock)
	{
		device->clock = calloc(1, sizeof(OSc_Clock));
		device->clock->device = device;
	}

	bool hasScanner;
	if (OSc_CHECK_ERROR(err, OSc_Device_HasScanner(device, &hasScanner)))
		goto Error;
	if (hasScanner)
	{
		device->scanner = calloc(1, sizeof(OSc_Scanner));
		device->scanner->device = device;
	}

	bool hasDetector;
	if (OSc_CHECK_ERROR(err, OSc_Device_HasDetector(device, &hasDetector)))
		goto Error;
	if (hasDetector)
	{
		device->detector = calloc(1, sizeof(OSc_Detector));
		device->detector->device = device;
	}

	if (OSc_CHECK_ERROR(err, OSc_LSM_Associate_Device(lsm, device)))
		goto Error;

	return OSc_Error_OK;

Error:
	OSc_Device_Close(device);
	return err;
}


OSc_Error OSc_Device_Close(OSc_Device *device)
{
	if (!device || !device->isOpen)
		return OSc_Error_OK;

	if (device->associatedLSM)
		OSc_RETURN_IF_ERROR(OSc_LSM_Dissociate_Device(device->associatedLSM, device));

	if (device->clock)
		free(device->clock);
	if (device->scanner)
		free(device->scanner);
	if (device->detector)
		free(device->detector);
	device->scanner = NULL;
	device->detector = NULL;

	OSc_RETURN_IF_ERROR(device->impl->Close(device));
	device->isOpen = false;

	return OSc_Error_OK;
}


OSc_Error OSc_Device_HasClock(OSc_Device *device, bool *hasClock)
{
	*hasClock = false;

	return device->impl->HasClock(device, hasClock);
}


OSc_Error OSc_Device_HasScanner(OSc_Device *device, bool *hasScanner)
{
	*hasScanner = false;

	return device->impl->HasScanner(device, hasScanner);
}


OSc_Error OSc_Device_HasDetector(OSc_Device *device, bool *hasDetector)
{
	*hasDetector = false;

	return device->impl->HasDetector(device, hasDetector);
}


OSc_Error OSc_Device_GetClock(OSc_Device *device, OSc_Clock **clock)
{
	*clock = NULL;

	bool hasClock = false;
	OSc_RETURN_IF_ERROR(OSc_Device_HasClock(device, &hasClock));
	if (!hasClock)
		return OSc_Error_Device_Does_Not_Support_Clock;

	*clock = device->clock;
	return OSc_Error_OK;
}


OSc_Error OSc_Device_GetScanner(OSc_Device *device, OSc_Scanner **scanner)
{
	*scanner = NULL;

	bool hasScanner = false;
	OSc_RETURN_IF_ERROR(OSc_Device_HasScanner(device, &hasScanner));
	if (!hasScanner)
		return OSc_Error_Device_Does_Not_Support_Scanner;

	*scanner = device->scanner;
	return OSc_Error_OK;
}


OSc_Error OSc_Device_GetDetector(OSc_Device *device, OSc_Detector **detector)
{
	*detector = NULL;

	bool hasDetector = false;
	OSc_RETURN_IF_ERROR(OSc_Device_HasDetector(device, &hasDetector));
	if (!hasDetector)
		return OSc_Error_Device_Does_Not_Support_Detector;

	*detector = device->detector;
	return OSc_Error_OK;
}


OSc_Error OSc_Device_GetSettings(OSc_Device *device, OSc_Setting ***settings, size_t *count)
{
	if (device->settings == NULL)
	{
		OSc_RETURN_IF_ERROR(device->impl->MakeSettings(device, &device->settings));
	}

	*settings = (OScDev_Setting**)device->settings->ptr;
	*count = device->settings->size;
	return OSc_Error_OK;
}


OSc_Error OSc_Device_GetAllowedResolutions(OSc_Device *device, size_t **widths, size_t **heights, size_t *count)
{
	OSc_RETURN_IF_ERROR(device->impl->GetAllowedResolutions(device, widths, heights, count));
	return OSc_Error_OK;
}


OSc_Error OSc_Device_GetResolution(OSc_Device *device, size_t *width, size_t *height)
{
	OSc_RETURN_IF_ERROR(device->impl->GetResolution(device, width, height));
	return OSc_Error_OK;
}


OSc_Error OSc_Device_SetResolution(OSc_Device *device, size_t width, size_t height)
{
	OSc_RETURN_IF_ERROR(device->impl->SetResolution(device, width, height));
	return OSc_Error_OK;
}

OSc_Error OSc_Device_GetMagnification(OSc_Device *device, double *magnification)
{
	OSc_RETURN_IF_ERROR(device->impl->GetMagnification(device, magnification));
	return OSc_Error_OK;
}


OSc_Error OSc_Device_SetMagnification(OSc_Device *device)
{
	OSc_RETURN_IF_ERROR(device->impl->SetMagnification(device));
	return OSc_Error_OK;
}


OSc_Error OSc_Device_Create(OSc_Device **device, OScDev_DeviceImpl *impl, void *data)
{
	*device = calloc(1, sizeof(OSc_Device));
	(*device)->impl = impl;
	(*device)->implData = data;
	return OSc_Error_OK;
}


OSc_Error OSc_Device_Destroy(OSc_Device *device)
{
	if (!device) {
		return OSc_Error_OK;
	}

	device->impl->ReleaseInstance(device);

	if (device->settings) {
		for (size_t i = 0; i < device->settings->size; ++i) {
			OSc_Setting *setting = device->settings->ptr[i];
			OSc_Setting_Destroy(setting);
		}
		OSc_PtrArray_Destroy(device->settings);
	}

	free(device);
	return OSc_Error_OK;
}
