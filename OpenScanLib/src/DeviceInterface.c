#include "DeviceInterface.h"
#include "OpenScanLibPrivate.h"

// This file contains thin wrappers around functions, to conform to the device
// interface. Please don't add any non-trivial logic here.


// TODO: Generalize to more than device; possibly accept impl ptrs
static void Log(OScDev_ModuleImpl *modImpl, OScDev_Device *device, OScDev_LogLevel dLevel, const char *message)
{
	// This only works because we define the enums identically:
	OSc_LogLevel level = (OSc_LogLevel)dLevel;

	OScInternal_Log(device, level, message);
}


static OScDev_PtrArray *PtrArray_Create(OScDev_ModuleImpl *modImpl)
{
	return OScInternal_PtrArray_Create();
}


static void PtrArray_Destroy(OScDev_ModuleImpl *modImpl, OScDev_PtrArray *arr)
{
	OScInternal_PtrArray_Destroy(arr);
}


static void PtrArray_Append(OScDev_ModuleImpl *modImpl, OScDev_PtrArray *arr, void *obj)
{
	OScInternal_PtrArray_Append(arr, obj);
}


static OScDev_NumArray *NumArray_Create(OScDev_ModuleImpl *modImpl)
{
	return OScInternal_NumArray_Create();
}


static void NumArray_Destroy(OScDev_ModuleImpl *modImpl, OScDev_NumArray *arr)
{
	OScInternal_NumArray_Destroy(arr);
}


static void NumArray_Append(OScDev_ModuleImpl *modImpl, OScDev_NumArray *arr, double val)
{
	OScInternal_NumArray_Append(arr, val);
}


static OScDev_NumRange *NumRange_CreateContinuous(OScDev_ModuleImpl *modImpl, double rMin, double rMax)
{
	return OScInternal_NumRange_CreateContinuous(rMin, rMax);
}


static OScDev_NumRange *NumRange_CreateDiscrete(OScDev_ModuleImpl *modImpl)
{
	return OScInternal_NumRange_CreateDiscrete();
}


static void NumRange_Destroy(OScDev_ModuleImpl *modImpl, OScDev_NumRange *range)
{
	OScInternal_NumRange_Destroy(range);
}


static void NumRange_AppendDiscrete(OScDev_ModuleImpl *modImpl, OScDev_NumRange *range, double val)
{
	OScInternal_NumRange_AppendDiscrete(range, val);
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
static OScDev_Error Setting_Create(OScDev_ModuleImpl *modImpl, OScDev_Setting **setting, const char *name, OScDev_ValueType valueType, OScDev_SettingImpl *impl, void *data)
{
	return OScInternal_Setting_Create(setting, name, valueType, impl, data);
}


static void *Setting_GetImplData(OScDev_ModuleImpl *modImpl, OScDev_Setting *setting)
{
	return OScInternal_Setting_GetImplData(setting);
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


static OScDev_Error Acquisition_GetClockStartTriggerSource(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq, OScDev_TriggerSource *startTrigger)
{
	// At this moment we don't yet support external start trigger.
	*startTrigger = OScDev_TriggerSource_Software;
	return OScDev_OK;
}


static OScDev_Error Acquisition_GetClockSource(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq, OScDev_ClockSource *clock)
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


static uint32_t Acquisition_GetNumberOfFrames(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq)
{
	OSc_Acquisition *acq = OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
	return OScInternal_Acquisition_GetNumberOfFrames(acq);
}


static double Acquisition_GetPixelRate(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq)
{
	OSc_Acquisition *acq = OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
	return OSc_Acquisition_GetPixelRate(acq);
}


static uint32_t Acquisition_GetResolution(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq)
{
	OSc_Acquisition *acq = OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
	return OSc_Acquisition_GetResolution(acq);
}


static double Acquisition_GetZoomFactor(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq)
{
	OSc_Acquisition *acq = OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
	return OSc_Acquisition_GetZoomFactor(acq);
}


static void Acquisition_GetROI(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *devAcq,
	uint32_t *xOffset, uint32_t *yOffset, uint32_t *width, uint32_t *height)
{
	OSc_Acquisition *acq = OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
	OSc_Acquisition_GetROI(acq, xOffset, yOffset, width, height);
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
	.Acquisition_IsClockRequested = Acquisition_IsClockRequested,
	.Acquisition_IsScannerRequested = Acquisition_IsScannerRequested,
	.Acquisition_IsDetectorRequested = Acquisition_IsDetectorRequested,
	.Acquisition_GetClockStartTriggerSource = Acquisition_GetClockStartTriggerSource,
	.Acquisition_GetClockSource = Acquisition_GetClockSource,
	.Acquisition_GetNumberOfFrames = Acquisition_GetNumberOfFrames,
	.Acquisition_GetPixelRate = Acquisition_GetPixelRate,
	.Acquisition_GetResolution = Acquisition_GetResolution,
	.Acquisition_GetZoomFactor = Acquisition_GetZoomFactor,
	.Acquisition_GetROI = Acquisition_GetROI,
	.Acquisition_CallFrameCallback = Acquisition_CallFrameCallback,
};
