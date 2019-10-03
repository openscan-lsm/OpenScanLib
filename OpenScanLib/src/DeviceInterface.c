#include "DeviceInterface.h"
#include "OpenScanLibPrivate.h"

// This file contains thin wrappers around functions, to conform to the device
// interface. Please don't add any non-trivial logic here.


// TODO: Generalize to more than device; possibly accept impl ptrs
static void Log(OScDev_ModuleImpl *modImpl, OScDev_Device *device, enum OScDev_LogLevel dLevel, const char *message)
{
	// This only works because we define the enums identically:
	OSc_LogLevel level = (OSc_LogLevel)dLevel;

	OSc_Log(device, level, message);
}


static OScDev_PtrArray *PtrArray_Create(OScDev_ModuleImpl *modImpl)
{
	return OSc_PtrArray_Create();
}


static void PtrArray_Destroy(OScDev_ModuleImpl *modImpl, OScDev_PtrArray *arr)
{
	OSc_PtrArray_Destroy(arr);
}


static void PtrArray_Append(OScDev_ModuleImpl *modImpl, OScDev_PtrArray *arr, void *obj)
{
	OSc_PtrArray_Append(arr, obj);
}


static OScDev_NumArray *NumArray_Create(OScDev_ModuleImpl *modImpl)
{
	return OSc_NumArray_Create();
}


static void NumArray_Destroy(OScDev_ModuleImpl *modImpl, OScDev_NumArray *arr)
{
	OSc_NumArray_Destroy(arr);
}


static void NumArray_Append(OScDev_ModuleImpl *modImpl, OScDev_NumArray *arr, double val)
{
	OSc_NumArray_Append(arr, val);
}


static OScDev_NumRange *NumRange_CreateContinuous(OScDev_ModuleImpl *modImpl, double rMin, double rMax)
{
	return OSc_NumRange_CreateContinuous(rMin, rMax);
}


static OScDev_NumRange *NumRange_CreateDiscrete(OScDev_ModuleImpl *modImpl)
{
	return OSc_NumRange_CreateDiscrete();
}


static void NumRange_Destroy(OScDev_ModuleImpl *modImpl, OScDev_NumRange *range)
{
	OSc_NumRange_Destroy(range);
}


static void NumRange_AppendDiscrete(OScDev_ModuleImpl *modImpl, OScDev_NumRange *range, double val)
{
	OSc_NumRange_AppendDiscrete(range, val);
}


// TODO: Replace with direct provision of impl and data (once we have DeviceLoaders)
static OScDev_Error Device_Create(OScDev_ModuleImpl *modImpl, OScDev_Device **device, OScDev_DeviceImpl *impl, void *data)
{
	return OScInternal_Device_Create(device, impl, data);
}


static void *Device_GetImplData(OScDev_ModuleImpl *modImpl, OScDev_Device *device)
{
	return OScInternal_Device_GetImplData(device);
}


// TODO: Replace with a SettingsCreateContext-based method
static OScDev_Error Setting_Create(OScDev_ModuleImpl *modImpl, OScDev_Setting **setting, const char *name, enum OScDev_ValueType valueType, OScDev_SettingImpl *impl, void *data)
{
	return OScInternal_Setting_Create(setting, name, valueType, impl, data);
}


static void *Setting_GetImplData(OScDev_ModuleImpl *modImpl, OScDev_Setting *setting)
{
	return OScInternal_Setting_GetImplData(setting);
}


static OScDev_Error Acquisition_GetNumberOfFrames(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq, uint32_t *numberOfFrames)
{
	OSc_Acquisition *acq = OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
	*numberOfFrames = OScInternal_Acquisition_GetNumberOfFrames(acq);
	return OScDev_OK;
}


static OScDev_Error Acquisition_IsClockRequested(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq, bool *isRequested)
{
	OSc_Device *device = OScInternal_AcquisitionForDevice_GetDevice(devAcq);
	OSc_Acquisition *acq = OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
	OSc_Device *clockDevice = OScInternal_Acquisition_GetClockDevice(acq);
	*isRequested = device == clockDevice;
	return OScDev_OK;
}


static OScDev_Error Acquisition_IsScannerRequested(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq, bool *isRequested)
{
	OSc_Device *device = OScInternal_AcquisitionForDevice_GetDevice(devAcq);
	OSc_Acquisition *acq = OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
	OSc_Device *scannerDevice = OScInternal_Acquisition_GetScannerDevice(acq);
	*isRequested = device == scannerDevice;
	return OScDev_OK;
}


static OScDev_Error Acquisition_IsDetectorRequested(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq, bool *isRequested)
{
	OSc_Device *device = OScInternal_AcquisitionForDevice_GetDevice(devAcq);
	OSc_Acquisition *acq = OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
	OSc_Device *detectorDevice = OScInternal_Acquisition_GetDetectorDevice(acq);
	*isRequested = device == detectorDevice;
	return OScDev_OK;
}


static OScDev_Error Acquisition_GetClockStartTriggerSource(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq, enum OScDev_TriggerSource *startTrigger)
{
	// At this moment we don't yet support external start trigger.
	*startTrigger = OScDev_TriggerSource_Software;
	return OScDev_OK;
}


static OScDev_Error Acquisition_GetClockSource(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq, enum OScDev_ClockSource *clock)
{
	OSc_Device *device = OScInternal_AcquisitionForDevice_GetDevice(devAcq);
	OSc_Acquisition *acq = OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
	OSc_Device *clockDevice = OScInternal_Acquisition_GetClockDevice(acq);

	if (device == clockDevice)
		*clock = OScDev_ClockSource_Internal;
	else
		*clock = OScDev_ClockSource_External;
	return OScDev_OK;
}


static bool Acquisition_CallFrameCallback(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq, uint32_t channel, void *pixels)
{
	OSc_Acquisition *acq = OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
	return OScInternal_Acquisition_CallFrameCallback(acq, channel, pixels);
}


struct OScDevInternal_Interface DeviceInterfaceFunctionTable = {
	.Log = Log,
	.PtrArray_Create = PtrArray_Create,
	.PtrArray_Destroy = PtrArray_Destroy,
	.PtrArray_Append = PtrArray_Append,
	.NumArray_Create = NumArray_Create,
	.NumArray_Destroy = NumArray_Destroy,
	.NumArray_Append = NumArray_Append,
	.NumRange_CreateContinuous = NumRange_CreateContinuous,
	.NumRange_CreateDiscrete = NumRange_CreateDiscrete,
	.NumRange_Destroy = NumRange_Destroy,
	.NumRange_AppendDiscrete = NumRange_AppendDiscrete,
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