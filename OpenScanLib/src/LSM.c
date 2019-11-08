#include "OpenScanLibPrivate.h"

#include <stdlib.h>
#include <string.h>


struct OScInternal_LSM
{
	OSc_Device *clockDevice;
	OSc_Device *scannerDevice;
	OSc_Device *detectorDevice;

	OSc_Device **associatedDevices;
	size_t associatedDeviceCount;
};


OSc_Error OSc_LSM_Create(OSc_LSM **lsm)
{
	*lsm = calloc(1, sizeof(OSc_LSM));
	return OSc_Error_OK;
}


OSc_Error OSc_LSM_Destroy(OSc_LSM *lsm)
{
	if (!lsm)
		return OSc_Error_OK;

	OSc_Error err;

	// We need to close each associated device, but doing so in turn dissociates
	// that device, so we need to make a copy of the list of associated devices
	// first.
	size_t nDevices = lsm->associatedDeviceCount;
	OSc_Device **devicesToClose = malloc(nDevices * sizeof(OSc_Device *));
	for (int i = 0; i < lsm->associatedDeviceCount; ++i)
	{
		devicesToClose[i] = lsm->associatedDevices[i];
	}

	for (int i = 0; i < nDevices; ++i)
	{
		OSc_Device *device = devicesToClose[i];
		if (OSc_CHECK_ERROR(err, OSc_Device_Close(device)))
		{
			char msg[OSc_MAX_STR_LEN + 1] = "Error while closing device ";
			const char *name = NULL;
			OSc_Device_GetName(device, &name);
			strcat(msg, name ? name : "(unknown)");
			OScInternal_LogError(device, msg);
		}
	}

	free(devicesToClose);
	free(lsm);
	return OSc_Error_OK;
}


OSc_Device *OSc_LSM_GetClockDevice(OSc_LSM *lsm)
{
	if (!lsm)
		return NULL;
	return lsm->clockDevice;
}


OSc_Device *OSc_LSM_GetScannerDevice(OSc_LSM *lsm)
{
	if (!lsm)
		return NULL;
	return lsm->scannerDevice;
}


OSc_Device *OSc_LSM_GetDetectorDevice(OSc_LSM *lsm)
{
	if (!lsm)
		return NULL;
	return lsm->detectorDevice;
}


OSc_Error OSc_LSM_SetClockDevice(OSc_LSM *lsm, OSc_Device *clockDevice)
{
	// TODO Should allow null device
	if (!lsm || !clockDevice)
		return OSc_Error_Illegal_Argument;

	OSc_Error err;
	bool isAssociated = false;
	if (OSc_CHECK_ERROR(err, OScInternal_LSM_Is_Device_Associated(lsm, clockDevice, &isAssociated)))
		return err;
	if (!isAssociated)
		return OSc_Error_Device_Not_Opened_For_LSM;

	lsm->clockDevice = clockDevice;
	return OSc_Error_OK;
}


OSc_Error OSc_LSM_SetScannerDevice(OSc_LSM *lsm, OSc_Device *scannerDevice)
{
	// TODO Should allow null device
	if (!lsm || !scannerDevice)
		return OSc_Error_Illegal_Argument;

	OSc_Error err;
	bool isAssociated = false;
	if (OSc_CHECK_ERROR(err, OScInternal_LSM_Is_Device_Associated(lsm, scannerDevice, &isAssociated)))
		return err;
	if (!isAssociated)
		return OSc_Error_Device_Not_Opened_For_LSM;

	lsm->scannerDevice = scannerDevice;
	return OSc_Error_OK;
}


OSc_Error OSc_LSM_SetDetectorDevice(OSc_LSM *lsm, OSc_Device *detectorDevice)
{
	// TODO Should allow null device
	if (!lsm || !detectorDevice)
		return OSc_Error_Illegal_Argument;

	OSc_Error err;
	bool isAssociated = false;
	if (OSc_CHECK_ERROR(err, OScInternal_LSM_Is_Device_Associated(lsm, detectorDevice, &isAssociated)))
		return err;
	if (!isAssociated)
		return OSc_Error_Device_Not_Opened_For_LSM;

	lsm->detectorDevice = detectorDevice;
	return OSc_Error_OK;
}


OSc_Error OScInternal_LSM_Associate_Device(OSc_LSM *lsm, OSc_Device *device)
{
	bool isAssociated = false;
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, OScInternal_LSM_Is_Device_Associated(lsm, device, &isAssociated)))
		return err;
	if (isAssociated)
		return OSc_Error_Device_Already_Open;

	if (!lsm->associatedDevices)
	{
		lsm->associatedDevices = malloc(sizeof(OSc_Device *));
		lsm->associatedDeviceCount = 1;
	}
	else {
		lsm->associatedDevices = realloc(lsm->associatedDevices,
			(++lsm->associatedDeviceCount) * sizeof(OSc_Device *));
	}
	lsm->associatedDevices[lsm->associatedDeviceCount - 1] = device;

	return OSc_Error_OK;
}


OSc_Error OScInternal_LSM_Dissociate_Device(OSc_LSM *lsm, OSc_Device *device)
{
	bool found = false;
	OSc_Device **newList = malloc(lsm->associatedDeviceCount * sizeof(OSc_Device *));
	for (int i = 0; i < lsm->associatedDeviceCount; ++i)
	{
		if (lsm->associatedDevices[i] == device)
		{
			found = true;
			continue;
		}
		newList[found ? i - 1 : i] = lsm->associatedDevices[i];
	}

	if (!found)
	{
		free(newList);
		return OSc_Error_Device_Not_Opened_For_LSM;
	}

	free(lsm->associatedDevices);
	lsm->associatedDevices = newList;
	--lsm->associatedDeviceCount;
	return OSc_Error_OK;
}


OSc_Error OScInternal_LSM_Is_Device_Associated(OSc_LSM *lsm, OSc_Device *device, bool *isAssociated)
{
	*isAssociated = false;
	for (int i = 0; i < lsm->associatedDeviceCount; ++i)
	{
		if (lsm->associatedDevices[i] == device)
		{
			*isAssociated = true;
			break;
		}
	}

	return OSc_Error_OK;
}


OSc_Error OSc_LSM_IsRunningAcquisition(OSc_LSM *lsm, bool *isRunning)
{
	*isRunning = false;
	for (int i = 0; i < lsm->associatedDeviceCount; ++i)
	{
		OSc_Device *device = lsm->associatedDevices[i];
		OSc_Error err;
		if (OSc_CHECK_ERROR(err, OScInternal_Device_IsRunning(device, isRunning)))
			return err;
		if (*isRunning)
			return OSc_Error_OK;
	}
	return OSc_Error_OK;
}