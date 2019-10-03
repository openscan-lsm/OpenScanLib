#include "OpenScanLibPrivate.h"

#include <stdlib.h>


struct OScInternal_AcquisitionForDevice
{
	OSc_Device *device;
	OSc_Acquisition *acq;
};


struct OScInternal_Acquisition
{
	OSc_Clock *clock;
	OSc_Scanner *scanner;
	OSc_Detector *detector;
	uint32_t numberOfFrames;
	OSc_FrameCallback frameCallback;
	void *data;

	// We can pass opaque pointers to these structs to devices, so that we can
	// handle acquisition-related calls in a device-specific manner.
	struct OScInternal_AcquisitionForDevice acqForClockDevice;
	struct OScInternal_AcquisitionForDevice acqForScannerDevice;
	struct OScInternal_AcquisitionForDevice acqForDetectorDevice;
};


OSc_Device *OScInternal_AcquisitionForDevice_GetDevice(OScDev_Acquisition *devAcq)
{
	return devAcq->device;
}


OSc_Acquisition *OScInternal_AcquisitionForDevice_GetAcquisition(OScDev_Acquisition *devAcq)
{
	return devAcq->acq;
}


OSc_Error OSc_Acquisition_Create(OSc_Acquisition **acq, OSc_LSM *lsm)
{
	*acq = calloc(1, sizeof(OSc_Acquisition));

	OSc_Clock *clock = NULL;
	OSc_LSM_GetClock(lsm, &clock);
	OSc_Scanner *scanner = NULL;
	OSc_LSM_GetScanner(lsm, &scanner);
	OSc_Detector *detector = NULL;
	OSc_LSM_GetDetector(lsm, &detector);
	
	(*acq)->clock = clock;
	(*acq)->scanner = scanner;
	(*acq)->detector = detector;
	(*acq)->numberOfFrames = 1;

	OSc_Device *clockDevice = NULL;
	OSc_Clock_GetDevice(clock, &clockDevice);
	OSc_Device *scannerDevice = NULL;
	OSc_Scanner_GetDevice(scanner, &scannerDevice);
	OSc_Device *detectorDevice = NULL;
	OSc_Detector_GetDevice(detector, &detectorDevice);

	(*acq)->acqForClockDevice.device = clockDevice;
	(*acq)->acqForClockDevice.acq = *acq;
	(*acq)->acqForScannerDevice.device = scannerDevice;
	(*acq)->acqForScannerDevice.acq = *acq;
	(*acq)->acqForDetectorDevice.device = detectorDevice;
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


uint32_t OScInternal_Acquisition_GetNumberOfFrames(OSc_Acquisition *acq)
{
	return acq->numberOfFrames;
}


OSc_Device *OScInternal_Acquisition_GetClockDevice(OSc_Acquisition *acq)
{
	OSc_Device *ret;
	OSc_Clock_GetDevice(acq->clock, &ret);
	return ret;
}


OSc_Device *OScInternal_Acquisition_GetScannerDevice(OSc_Acquisition *acq)
{
	OSc_Device *ret;
	OSc_Scanner_GetDevice(acq->scanner, &ret);
	return ret;
}


OSc_Device *OScInternal_Acquisition_GetDetectorDevice(OSc_Acquisition *acq)
{
	OSc_Device *ret;
	OSc_Detector_GetDevice(acq->detector, &ret);
	return ret;
}


bool OScInternal_Acquisition_CallFrameCallback(OSc_Acquisition *acq, uint32_t channel, void *pixels)
{
	if (acq->frameCallback == NULL) {
		return true;
	}

	return acq->frameCallback(acq, channel, pixels, acq->data);
}