#include "OpenScanLibPrivate.h"

#include <stdlib.h>


struct OScInternal_AcquisitionForDevice
{
	OSc_Device *device;
	OSc_Acquisition *acq;
};


struct OScInternal_Acquisition
{
	OSc_Device *clockDevice;
	OSc_Device *scannerDevice;
	OSc_Device *detectorDevice;
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

	(*acq)->clockDevice = OSc_LSM_GetClockDevice(lsm);
	(*acq)->scannerDevice = OSc_LSM_GetScannerDevice(lsm);
	(*acq)->detectorDevice = OSc_LSM_GetDetectorDevice(lsm);
	(*acq)->numberOfFrames = 1;

	(*acq)->acqForClockDevice.device = (*acq)->clockDevice;
	(*acq)->acqForClockDevice.acq = *acq;
	(*acq)->acqForScannerDevice.device = (*acq)->scannerDevice;
	(*acq)->acqForScannerDevice.acq = *acq;
	(*acq)->acqForDetectorDevice.device = (*acq)->detectorDevice;
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
		acq->clockDevice->impl->Arm(acq->clockDevice,
			&acq->acqForClockDevice)))
		return err;

	// Scanner, if different device
	if (acq->scannerDevice != acq->clockDevice)
	{
		if (OSc_CHECK_ERROR(err,
			acq->scannerDevice->impl->Arm(acq->scannerDevice,
				&acq->acqForScannerDevice)))
		{
			acq->clockDevice->impl->Stop(acq->clockDevice);
			return err;
		}
	}

	// Detector, if different device
	if (acq->detectorDevice != acq->clockDevice &&
		acq->detectorDevice != acq->scannerDevice)
	{
		if (OSc_CHECK_ERROR(err,
			acq->detectorDevice->impl->Arm(acq->detectorDevice,
				&acq->acqForDetectorDevice)))
		{
			acq->scannerDevice->impl->Stop(acq->scannerDevice);
			if (acq->clockDevice != acq->scannerDevice)
				acq->clockDevice->impl->Stop(acq->clockDevice);
			return err;
		}
	}

	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_Start(OSc_Acquisition *acq)
{
	// TODO Error if not armed
	return acq->clockDevice->impl->Start(acq->clockDevice);
}


OSc_Error OSc_Acquisition_Stop(OSc_Acquisition *acq)
{
	// Stop() is idempotent, so we don't bother to determine the unique devices
	acq->clockDevice->impl->Stop(acq->clockDevice);
	acq->scannerDevice->impl->Stop(acq->scannerDevice);
	acq->detectorDevice->impl->Stop(acq->detectorDevice);

	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_Wait(OSc_Acquisition *acq)
{
	acq->clockDevice->impl->Wait(acq->clockDevice);
	acq->scannerDevice->impl->Wait(acq->scannerDevice);
	acq->detectorDevice->impl->Wait(acq->detectorDevice);
	return OSc_Error_OK;
}


uint32_t OScInternal_Acquisition_GetNumberOfFrames(OSc_Acquisition *acq)
{
	return acq->numberOfFrames;
}


OSc_Device *OScInternal_Acquisition_GetClockDevice(OSc_Acquisition *acq)
{
	return acq->clockDevice;
}


OSc_Device *OScInternal_Acquisition_GetScannerDevice(OSc_Acquisition *acq)
{
	return acq->scannerDevice;
}


OSc_Device *OScInternal_Acquisition_GetDetectorDevice(OSc_Acquisition *acq)
{
	return acq->detectorDevice;
}


bool OScInternal_Acquisition_CallFrameCallback(OSc_Acquisition *acq, uint32_t channel, void *pixels)
{
	if (acq->frameCallback == NULL) {
		return true;
	}

	return acq->frameCallback(acq, channel, pixels, acq->data);
}