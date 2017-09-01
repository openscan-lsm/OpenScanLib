#include "OpenScanLibPrivate.h"
#include "OpenScanDeviceImpl.h"

#include <stdlib.h>
#include <string.h>


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

	for (int i = 0; i < lsm->associatedDeviceCount; ++i)
	{
		OSc_Device *device = lsm->associatedDevices[i];
		if (OSc_Check_Error(err, OSc_Device_Close(device)))
		{
			char msg[OSc_MAX_STR_LEN + 1] = "Error while closing device ";
			const char *name = NULL;
			OSc_Device_Get_Name(device, &name);
			strcat(msg, name ? name : "(unknown)");
			OSc_Log_Error(lsm->associatedDevices[i], msg);
		}
	}

	free(lsm);
	return OSc_Error_OK;
}


OSc_Error OSc_LSM_Get_Scanner(OSc_LSM *lsm, OSc_Scanner **scanner)
{
	if (!lsm || !scanner)
		return OSc_Error_Illegal_Argument;

	*scanner = lsm->scanner;
	return OSc_Error_OK;
}


OSc_Error OSc_LSM_Set_Scanner(OSc_LSM *lsm, OSc_Scanner *scanner)
{
	if (!lsm || !scanner)
		return OSc_Error_Illegal_Argument;

	OSc_Device *scannerDevice;
	OSc_Return_If_Error(OSc_Scanner_Get_Device(scanner, &scannerDevice));

	bool isAssociated = false;
	OSc_Return_If_Error(OSc_LSM_Is_Device_Associated(lsm, scannerDevice, &isAssociated));
	if (!isAssociated)
		return OSc_Error_Device_Not_Opened_For_LSM;

	lsm->scanner = scanner;
	return OSc_Error_OK;
}


OSc_Error OSc_LSM_Get_Detector(OSc_LSM *lsm, OSc_Detector **detector)
{
	if (!lsm || !detector)
		return OSc_Error_Illegal_Argument;

	*detector = lsm->detector;
	return OSc_Error_OK;
}


OSc_Error OSc_LSM_Set_Detector(OSc_LSM *lsm, OSc_Detector *detector)
{
	if (!lsm || !detector)
		return OSc_Error_Illegal_Argument;

	OSc_Device *detectorDevice;
	OSc_Return_If_Error(OSc_Detector_Get_Device(detector, &detectorDevice));

	bool isAssociated = false;
	OSc_Return_If_Error(OSc_LSM_Is_Device_Associated(lsm, detectorDevice, &isAssociated));
	if (!isAssociated)
		return OSc_Error_Device_Not_Opened_For_LSM;

	lsm->detector = detector;
	return OSc_Error_OK;
}


OSc_Error OSc_LSM_Associate_Device(OSc_LSM *lsm, OSc_Device *device)
{
	bool isAssociated = false;
	OSc_Return_If_Error(OSc_LSM_Is_Device_Associated(lsm, device, &isAssociated));
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


OSc_Error OSc_LSM_Dissociate_Device(OSc_LSM *lsm, OSc_Device *device)
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


OSc_Error OSc_LSM_Is_Device_Associated(OSc_LSM *lsm, OSc_Device *device, bool *isAssociated)
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


OSc_Error OSc_LSM_Is_Running_Acquisition(OSc_LSM *lsm, bool *isRunning)
{
	*isRunning = false;
	for (int i = 0; i < lsm->associatedDeviceCount; ++i)
	{
		OSc_Device *device = lsm->associatedDevices[i];
		OSc_Return_If_Error(device->impl->IsRunning(device, isRunning));
		if (isRunning)
			return OSc_Error_OK;
	}
	return OSc_Error_OK;
}