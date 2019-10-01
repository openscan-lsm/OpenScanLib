#include "OpenScanLibPrivate.h"


OSc_Error OSc_Detector_GetDevice(OSc_Detector *detector, OSc_Device **device)
{
	*device = detector->device;
	return OSc_Error_OK;
}


OSc_Error OSc_Detector_GetImageSize(OSc_Detector *detector, uint32_t *width, uint32_t *height)
{
	return detector->device->impl->GetImageSize(detector->device, width, height);
}


OSc_Error OSc_Detector_GetNumberOfChannels(OSc_Detector *detector, uint32_t *nChannels)
{
	return detector->device->impl->GetNumberOfChannels(detector->device, nChannels);
}


OSc_Error OSc_Detector_GetBytesPerSample(OSc_Detector *detector, uint32_t *bytesPerSample)
{
	return detector->device->impl->GetBytesPerSample(detector->device, bytesPerSample);
}