#include "OpenScanLibPrivate.h"

#include <stdlib.h>
#include <string.h>


struct OScInternal_LSM
{
	OSc_Clock *clock;
	OSc_Scanner *scanner;
	OSc_Detector *detector;

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
	OSc_Device **devicesToClose = malloc(nDevices * sizeof(OSc_Device));
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
			OSc_Log_Error(device, msg);
		}
	}

	free(devicesToClose);
	free(lsm);
	return OSc_Error_OK;
}


OSc_Error OSc_LSM_GetClock(OSc_LSM *lsm, OSc_Clock **clock)
{
	if (!lsm || !clock)
		return OSc_Error_Illegal_Argument;

	*clock = lsm->clock;
	return OSc_Error_OK;
}


OSc_Error OSc_LSM_SetClock(OSc_LSM *lsm, OSc_Clock *clock)
{
	if (!lsm || !clock)
		return OSc_Error_Illegal_Argument;

	OSc_Device *clockDevice;
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, OSc_Clock_GetDevice(clock, &clockDevice)))
		return err;

	bool isAssociated = false;
	if (OSc_CHECK_ERROR(err, OScInternal_LSM_Is_Device_Associated(lsm, clockDevice, &isAssociated)))
		return err;
	if (!isAssociated)
		return OSc_Error_Device_Not_Opened_For_LSM;

	lsm->clock = clock;
	return OSc_Error_OK;
}


OSc_Error OSc_LSM_GetScanner(OSc_LSM *lsm, OSc_Scanner **scanner)
{
	if (!lsm || !scanner)
		return OSc_Error_Illegal_Argument;

	*scanner = lsm->scanner;
	return OSc_Error_OK;
}


OSc_Error OSc_LSM_SetScanner(OSc_LSM *lsm, OSc_Scanner *scanner)
{
	if (!lsm || !scanner)
		return OSc_Error_Illegal_Argument;

	OSc_Device *scannerDevice;
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, OSc_Scanner_GetDevice(scanner, &scannerDevice)))
		return err;

	bool isAssociated = false;
	if (OSc_CHECK_ERROR(err, OScInternal_LSM_Is_Device_Associated(lsm, scannerDevice, &isAssociated)))
		return err;
	if (!isAssociated)
		return OSc_Error_Device_Not_Opened_For_LSM;

	lsm->scanner = scanner;
	return OSc_Error_OK;
}


OSc_Error OSc_LSM_GetDetector(OSc_LSM *lsm, OSc_Detector **detector)
{
	if (!lsm || !detector)
		return OSc_Error_Illegal_Argument;

	*detector = lsm->detector;
	return OSc_Error_OK;
}


OSc_Error OSc_LSM_SetDetector(OSc_LSM *lsm, OSc_Detector *detector)
{
	if (!lsm || !detector)
		return OSc_Error_Illegal_Argument;

	OSc_Device *detectorDevice;
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, OSc_Detector_GetDevice(detector, &detectorDevice)))
		return err;

	bool isAssociated = false;
	if (OSc_CHECK_ERROR(err, OScInternal_LSM_Is_Device_Associated(lsm, detectorDevice, &isAssociated)))
		return err;
	if (!isAssociated)
		return OSc_Error_Device_Not_Opened_For_LSM;

	lsm->detector = detector;
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
		lsm->associatedDevices = malloc(sizeof(OSc_Device));
		lsm->associatedDeviceCount = 1;
	}
	else {
		lsm->associatedDevices = realloc(lsm->associatedDevices,
			(++lsm->associatedDeviceCount) * sizeof(OSc_Device));
	}
	lsm->associatedDevices[lsm->associatedDeviceCount - 1] = device;

	device->associatedLSM = lsm;

	return OSc_Error_OK;
}


OSc_Error OScInternal_LSM_Dissociate_Device(OSc_LSM *lsm, OSc_Device *device)
{
	bool found = false;
	OSc_Device **newList = malloc(lsm->associatedDeviceCount * sizeof(OSc_Device));
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
		if (OSc_CHECK_ERROR(err, device->impl->IsRunning(device, isRunning)))
			return err;
		if (*isRunning)
			return OSc_Error_OK;
	}
	return OSc_Error_OK;
}