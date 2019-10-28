#include "OpenScanLibPrivate.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


struct OScInternal_Device
{
	OScDev_DeviceImpl *impl;
	void *implData;

	OSc_LogFunc logFunc;
	void *logData;

	bool isOpen;

	OSc_LSM *associatedLSM;

	OScInternal_PtrArray *settings;

	char name[OSc_MAX_STR_LEN + 1];
	char displayName[OSc_MAX_STR_LEN + 1];
};


void OSc_Device_SetLogFunc(OSc_Device *device, OSc_LogFunc func, void *data)
{
	if (!device)
		return;

	device->logFunc = func;
	device->logData = data;
}


OSc_Error OSc_Device_GetName(OSc_Device *device, const char **name)
{
	OSc_Error err;
	if (!strlen(device->name))
	{
		if (OSc_CHECK_ERROR(err, device->impl->GetName(device, device->name)))
			return err;
	}

	*name = device->name;
	return OSc_Error_OK;
}


OSc_Error OSc_Device_GetDisplayName(OSc_Device *device, const char **name)
{
	if (!strlen(device->displayName))
	{
		OSc_Error err;
		const char *modelName;
		if (OSc_CHECK_ERROR(err, device->impl->GetModelName(&modelName)))
			return err;
		const char *deviceName;
		if (OSc_CHECK_ERROR(err, OSc_Device_GetName(device, &deviceName)))
			return err;
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

	OSc_Error err;
	if (OSc_CHECK_ERROR(err, device->impl->Open(device)))
		return err;
	device->isOpen = true;

	if (OSc_CHECK_ERROR(err, OScInternal_LSM_Associate_Device(lsm, device)))
		goto Error;
	device->associatedLSM = lsm;

	return OSc_Error_OK;

Error:
	OSc_Device_Close(device);
	return err;
}


OSc_Error OSc_Device_Close(OSc_Device *device)
{
	if (!device || !device->isOpen)
		return OSc_Error_OK;

	OSc_Error err;
	if (device->associatedLSM)
	{
		if (OSc_CHECK_ERROR(err, OScInternal_LSM_Dissociate_Device(device->associatedLSM, device)))
			return err;
	}

	if (OSc_CHECK_ERROR(err, device->impl->Close(device)))
		return err;
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


OSc_Error OSc_Device_GetSettings(OSc_Device *device, OSc_Setting ***settings, size_t *count)
{
	if (device->settings == NULL)
	{
		OSc_Error err;
		if (OSc_CHECK_ERROR(err, device->impl->MakeSettings(device, &device->settings)))
			return err;
	}

	*settings = (OScDev_Setting**)OScInternal_PtrArray_Data(device->settings);
	*count = OScInternal_PtrArray_Size(device->settings);
	return OSc_Error_OK;
}


OSc_Error OScInternal_Device_Create(OSc_Device **device, OScDev_DeviceImpl *impl, void *data)
{
	*device = calloc(1, sizeof(OSc_Device));
	(*device)->impl = impl;
	(*device)->implData = data;
	return OSc_Error_OK;
}


OSc_Error OScInternal_Device_Destroy(OSc_Device *device)
{
	if (!device) {
		return OSc_Error_OK;
	}

	device->impl->ReleaseInstance(device);

	if (device->settings) {
		size_t nSettings = OScInternal_PtrArray_Size(device->settings);
		for (size_t i = 0; i < nSettings; ++i) {
			OSc_Setting *setting = OScInternal_PtrArray_At(device->settings, i);
			OScInternal_Setting_Destroy(setting);
		}
		OScInternal_PtrArray_Destroy(device->settings);
	}

	free(device);
	return OSc_Error_OK;
}


bool OScInternal_Device_Log(OSc_Device *device, OSc_LogLevel level, const char *message)
{
	if (!device || !device->logFunc)
		return false;
	device->logFunc(message, level, device->logData);
	return true;
}


void *OScInternal_Device_GetImplData(OSc_Device *device)
{
	if (!device)
		return NULL;
	return device->implData;
}


OScDev_NumRange *OScInternal_Device_GetPixelRates(OSc_Device *device)
{
	if (!device)
		return NULL;
	if (device->impl->GetPixelRates) {
		OScDev_NumRange *ret;
		OSc_Error err;
		if (OSc_CHECK_ERROR(err, device->impl->GetPixelRates(device, &ret))) {
			return OScInternal_NumRange_CreateDiscrete();
		}
		return ret;
	}
	return OScInternal_NumRange_CreateContinuous(1e-3, 1e10);
}


OScDev_NumRange *OScInternal_Device_GetResolutions(OSc_Device *device)
{
	if (!device)
		return NULL;
	if (device->impl->GetResolutions) {
		OScDev_NumRange *ret;
		OSc_Error err;
		if (OSc_CHECK_ERROR(err, device->impl->GetResolutions(device, &ret))) {
			return OScInternal_NumRange_CreateDiscrete();
		}
		return ret;
	}
	return OScInternal_NumRange_CreateContinuous(1, INT32_MAX);
}


OScDev_NumRange *OScInternal_Device_GetZooms(OSc_Device *device)
{
	if (!device)
		return NULL;
	if (device->impl->GetZoomFactors) {
		OScDev_NumRange *ret;
		OSc_Error err;
		if (OSc_CHECK_ERROR(err, device->impl->GetZoomFactors(device, &ret))) {
			return OScInternal_NumRange_CreateDiscrete();
		}
		return ret;
	}
	return OScInternal_NumRange_CreateContinuous(1e-6, 1e6);
}


bool OScInternal_Device_IsROIScanSupported(OSc_Device *device)
{
	if (!device)
		return false;
	if (!device->impl->IsROIScanSupported) {
		return false;
	}

	bool supported;
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, device->impl->IsROIScanSupported(device, &supported))) {
		return false;
	}
	return supported;
}


OScInternal_NumRange *OScInternal_Device_GetRasterWidths(OSc_Device *device)
{
	if (!device)
		return NULL;
	if (device->impl->GetRasterWidths) {
		OScDev_NumRange *ret;
		OSc_Error err;
		if (OSc_CHECK_ERROR(err, device->impl->GetRasterWidths(device, &ret))) {
			return OScInternal_NumRange_CreateDiscrete();
		}
		return ret;
	}
	return OScInternal_NumRange_CreateContinuous(1, INT32_MAX);
}


OScInternal_NumRange *OScInternal_Device_GetRasterHeights(OSc_Device *device)
{
	if (!device)
		return NULL;
	if (device->impl->GetRasterHeights) {
		OScDev_NumRange *ret;
		OSc_Error err;
		if (OSc_CHECK_ERROR(err, device->impl->GetRasterHeights(device, &ret))) {
			return OScInternal_NumRange_CreateDiscrete();
		}
		return ret;
	}
	return OScInternal_NumRange_CreateContinuous(1, INT32_MAX);
}


OSc_Error OScInternal_Device_GetNumberOfChannels(OSc_Device *device, uint32_t *numberOfChannels)
{
	if (!device || !numberOfChannels)
		return OSc_Error_Illegal_Argument;
	if (device->impl->GetNumberOfChannels) {
		return device->impl->GetNumberOfChannels(device, numberOfChannels);
	}
	*numberOfChannels = 0;
	return OSc_Error_Device_Does_Not_Support_Detector;
}


OSc_Error OScInternal_Device_GetBytesPerSample(OSc_Device *device, uint32_t *bytesPerSample)
{
	if (!device || !bytesPerSample)
		return OSc_Error_Illegal_Argument;
	if (device->impl->GetBytesPerSample) {
		return device->impl->GetBytesPerSample(device, bytesPerSample);
	}
	*bytesPerSample = 0;
	return OSc_Error_Device_Does_Not_Support_Detector;
}


OSc_Error OScInternal_Device_Arm(OSc_Device *device, OSc_Acquisition *acq)
{
	if (!device || !acq)
		return OSc_Error_Illegal_Argument;

	return device->impl->Arm(device, OScInternal_Acquisition_GetForDevice(acq, device));
}


OSc_Error OScInternal_Device_Start(OSc_Device *device)
{
	if (!device)
		return OSc_Error_Illegal_Argument;

	return device->impl->Start(device);
}


void OScInternal_Device_Stop(OSc_Device *device)
{
	if (!device)
		return;

	device->impl->Stop(device);
}


void OScInternal_Device_Wait(OSc_Device *device)
{
	if (!device)
		return;

	device->impl->Wait(device);
}


OSc_Error OScInternal_Device_IsRunning(OSc_Device *device, bool *isRunning)
{
	if (!device || !isRunning)
		return OSc_Error_Illegal_Argument;

	return device->impl->IsRunning(device, isRunning);
}
