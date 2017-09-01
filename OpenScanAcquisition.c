#include "OpenScanLibPrivate.h"
#include "OpenScanDeviceImpl.h"


OSc_Error OSc_Acquisition_Create(OSc_Acquisition **acq, OSc_LSM *lsm)
{
	*acq = calloc(1, sizeof(OSc_Acquisition));

	// TODO Use dummy for null scanner or detector?

	(*acq)->scanner = lsm->scanner;
	(*acq)->detector = lsm->detector;
	(*acq)->numberOfFrames = 1;
	(*acq)->triggerSource = OSc_Trigger_Source_External;
	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_Destroy(OSc_Acquisition *acq)
{
	free(acq);
	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_Set_Number_Of_Frames(OSc_Acquisition *acq, uint32_t numberOfFrames)
{
	acq->numberOfFrames = numberOfFrames;
	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_Set_Trigger_Source(OSc_Acquisition *acq, OSc_Trigger_Source source)
{
	acq->triggerSource = source;
	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_Set_Frame_Callback(OSc_Acquisition *acq, OSc_Frame_Callback callback)
{
	acq->frameCallback = callback;
	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_Get_Data(OSc_Acquisition *acq, void **data)
{
	*data = acq->data;
	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_Set_Data(OSc_Acquisition *acq, void *data)
{
	acq->data = data;
	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_Arm(OSc_Acquisition *acq)
{
	if (acq->triggerSource == OSc_Trigger_Source_Scanner ||
		acq->triggerSource == OSc_Trigger_Source_External)
	{
		acq->detector->device->impl->ArmDetector(acq->detector->device, acq);
	}

	if (acq->triggerSource == OSc_Trigger_Source_Detector ||
		acq->triggerSource == OSc_Trigger_Source_External)
	{
		acq->scanner->device->impl->ArmScanner(acq->scanner->device, acq);
	}

	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_Start(OSc_Acquisition *acq)
{
	if (acq->triggerSource == OSc_Trigger_Source_Detector ||
		acq->triggerSource == OSc_Trigger_Source_External)
	{
		acq->detector->device->impl->StartDetector(acq->detector->device, acq);
	}

	if (acq->triggerSource == OSc_Trigger_Source_Scanner ||
		acq->triggerSource == OSc_Trigger_Source_External)
	{
		acq->scanner->device->impl->StartScanner(acq->scanner->device, acq);
	}

	return OSc_Error_OK;
}


OSc_Error OSc_Acquisition_Stop(OSc_Acquisition *acq)
{
	acq->scanner->device->impl->StopScanner(acq->scanner->device, acq);
	acq->detector->device->impl->StopDetector(acq->detector->device, acq);

	return OSc_Error_OK;
}