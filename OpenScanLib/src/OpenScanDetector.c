#include "OpenScanLibPrivate.h"
#include "OpenScanDeviceImpl.h"


OSc_Error OSc_Detector_Get_Device(OSc_Detector *detector, OSc_Device **device)
{
	*device = detector->device;
	return OSc_Error_OK;
}


OSc_Error OSc_Detector_Get_Image_Size(OSc_Detector *detector, uint32_t *width, uint32_t *height)
{
	return detector->device->impl->GetImageSize(detector->device, width, height);
}


OSc_Error OSc_Detector_Get_Number_Of_Channels(OSc_Detector *detector, uint32_t *nChannels)
{
	return detector->device->impl->GetNumberOfChannels(detector->device, nChannels);
}


OSc_Error OSc_Detector_Get_Bytes_Per_Sample(OSc_Detector *detector, uint32_t *bytesPerSample)
{
	return detector->device->impl->GetBytesPerSample(detector->device, bytesPerSample);
}