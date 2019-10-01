#include "OpenScanLibPrivate.h"

#include <stdlib.h>


OSc_Error OSc_Acquisition_Create(OSc_Acquisition **acq, OSc_LSM *lsm)
{
	*acq = calloc(1, sizeof(OSc_Acquisition));

	// TODO Use dummy for null scanner or detector?

	(*acq)->clock = lsm->clock;
	(*acq)->scanner = lsm->scanner;
	(*acq)->detector = lsm->detector;
	(*acq)->numberOfFrames = 1;

	(*acq)->acqForClockDevice.device = lsm->clock->device;
	(*acq)->acqForClockDevice.acq = *acq;
	(*acq)->acqForScannerDevice.device = lsm->scanner->device;
	(*acq)->acqForScannerDevice.acq = *acq;
	(*acq)->acqForDetectorDevice.device = lsm->detector->device;
	(*acq)->acqForDetectorDevice.acq = *acq;

	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_Destroy(OSc_Acquisition *acq)
{
	free(acq);
	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_SetNumberOfFrames(OSc_Acquisition *acq, uint32_t numberOfFrames)
{
	acq->numberOfFrames = numberOfFrames;
	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_SetFrameCallback(OSc_Acquisition *acq, OSc_FrameCallback callback)
{
	acq->frameCallback = callback;
	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_GetData(OSc_Acquisition *acq, void **data)
{
	*data = acq->data;
	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_SetData(OSc_Acquisition *acq, void *data)
{
	acq->data = data;
	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_Arm(OSc_Acquisition *acq)
{
	// Arm each device participating in the acquisition exactly once each

	OSc_Error err;

	// Clock
	if (OSc_CHECK_ERROR(err,
		acq->clock->device->impl->Arm(acq->clock->device,
			&acq->acqForClockDevice)))
		return err;

	// Scanner, if different device
	if (acq->scanner->device != acq->clock->device)
	{
		if (OSc_CHECK_ERROR(err,
			acq->scanner->device->impl->Arm(acq->scanner->device,
				&acq->acqForScannerDevice)))
		{
			acq->clock->device->impl->Stop(acq->clock->device);
			return err;
		}
	}

	// Detector, if different device
	if (acq->detector->device != acq->clock->device &&
		acq->detector->device != acq->scanner->device)
	{
		if (OSc_CHECK_ERROR(err,
			acq->detector->device->impl->Arm(acq->detector->device,
				&acq->acqForDetectorDevice)))
		{
			acq->scanner->device->impl->Stop(acq->scanner->device);
			if (acq->clock->device != acq->scanner->device)
				acq->clock->device->impl->Stop(acq->clock->device);
			return err;
		}
	}

	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_Start(OSc_Acquisition *acq)
{
	// TODO Error if not armed
	return acq->clock->device->impl->Start(acq->clock->device);
}


OSc_Error OSc_Acquisition_Stop(OSc_Acquisition *acq)
{
	// Stop() is idempotent, so we don't bother to determine the unique devices
	acq->clock->device->impl->Stop(acq->clock->device);
	acq->scanner->device->impl->Stop(acq->scanner->device);
	acq->detector->device->impl->Stop(acq->detector->device);

	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_Wait(OSc_Acquisition *acq)
{
	acq->clock->device->impl->Wait(acq->clock->device);
	acq->scanner->device->impl->Wait(acq->scanner->device);
	acq->detector->device->impl->Wait(acq->detector->device);
	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_GetNumberOfFrames(OSc_Acquisition *acq, uint32_t *numberOfFrames)
{
	*numberOfFrames = acq->numberOfFrames;
	return OSc_Error_OK;
}