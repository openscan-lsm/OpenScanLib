#include "DeviceInterface.h"
#include "OpenScanLibPrivate.h"

// This file contains thin wrappers around functions, to conform to the device
// interface. Please don't add any non-trivial logic here.


// TODO: Generalize to more than device; possibly accept impl ptrs
static void Log(struct OScDev_ModuleImpl *modImpl, OScDev_Device *device, enum OScDev_LogLevel dLevel, const char *message)
{
	// This only works because we define the enums identically:
	OSc_Log_Level level = (OSc_Log_Level)dLevel;

	OSc_Log(device, level, message);
}


static OScDev_PtrArray *PtrArray_Create(struct OScDev_ModuleImpl *modImpl)
{
	return OSc_PtrArray_Create();
}


static void PtrArray_Destroy(struct OScDev_ModuleImpl *modImpl, OScDev_PtrArray *arr)
{
	OSc_PtrArray_Destroy(arr);
}


static void PtrArray_Append(struct OScDev_ModuleImpl *modImpl, OScDev_PtrArray *arr, void *obj)
{
	OSc_PtrArray_Append(arr, obj);
}


// TODO: Replace with direct provision of impl and data (once we have DeviceLoaders)
static OScDev_Error Device_Create(struct OScDev_ModuleImpl *modImpl, OScDev_Device **device, struct OScDev_DeviceImpl *impl, void *data)
{
	return OSc_Device_Create(device, impl, data);
}


static void *Device_GetImplData(struct OScDev_ModuleImpl *modImpl, OScDev_Device *device)
{
	return device->implData;
}


// TODO: Replace with a SettingsCreateContext-based method
static OScDev_Error Setting_Create(struct OScDev_ModuleImpl *modImpl, OScDev_Setting **setting, const char *name, enum OScDev_ValueType valueType, struct OScDev_SettingImpl *impl, void *data)
{
	return OSc_Setting_Create(setting, name, valueType, impl, data);
}


static void *Setting_GetImplData(struct OScDev_ModuleImpl *modImpl, OScDev_Setting *setting)
{
	return setting->implData;
}


static OScDev_Error Acquisition_GetNumberOfFrames(struct OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq, uint32_t *numberOfFrames)
{
	*numberOfFrames = devAcq->acq->numberOfFrames;
	return OScDev_OK;
}


static OScDev_Error Acquisition_IsClockRequested(struct OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq, bool *isRequested)
{
	*isRequested = devAcq->device == devAcq->acq->clock->device;
	return OScDev_OK;
}


static OScDev_Error Acquisition_IsScannerRequested(struct OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq, bool *isRequested)
{
	*isRequested = devAcq->device == devAcq->acq->scanner->device;
	return OScDev_OK;
}


static OScDev_Error Acquisition_IsDetectorRequested(struct OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq, bool *isRequested)
{
	*isRequested = devAcq->device == devAcq->acq->detector->device;
	return OScDev_OK;
}


static OScDev_Error Acquisition_GetClockStartTriggerSource(struct OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq, enum OScDev_TriggerSource *startTrigger)
{
	// At this moment we don't yet support external start trigger.
	*startTrigger = OScDev_TriggerSource_Software;
	return OScDev_OK;
}


static OScDev_Error Acquisition_GetClockSource(struct OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq, enum OScDev_ClockSource *clock)
{
	if (devAcq->device == devAcq->acq->clock->device)
		*clock = OScDev_ClockSource_Internal;
	else
		*clock = OScDev_ClockSource_External;
	return OScDev_OK;
}


static bool Acquisition_CallFrameCallback(struct OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq, uint32_t channel, void *pixels)
{
	return devAcq->acq->frameCallback(devAcq->acq, channel, pixels, devAcq->acq->data);
}


struct OScDevInternal_Interface DeviceInterfaceFunctionTable = {
	.Log = Log,
	.PtrArray_Create = PtrArray_Create,
	.PtrArray_Destroy = PtrArray_Destroy,
	.PtrArray_Append = PtrArray_Append,
	.Device_Create = Device_Create,
	.Device_GetImplData = Device_GetImplData,
	.Setting_Create = Setting_Create,
	.Setting_GetImplData = Setting_GetImplData,
	.Acquisition_GetNumberOfFrames = Acquisition_GetNumberOfFrames,
	.Acquisition_IsClockRequested = Acquisition_IsClockRequested,
	.Acquisition_IsScannerRequested = Acquisition_IsScannerRequested,
	.Acquisition_IsDetectorRequested = Acquisition_IsDetectorRequested,
	.Acquisition_GetClockStartTriggerSource = Acquisition_GetClockStartTriggerSource,
	.Acquisition_GetClockSource = Acquisition_GetClockSource,
	.Acquisition_CallFrameCallback = Acquisition_CallFrameCallback,
};