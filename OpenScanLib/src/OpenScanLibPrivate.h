#pragma once

#include "OpenScanLib.h"
#include "OpenScanDeviceLib.h"


// Internal functions

void OScInternal_Log(OSc_Device *device, OSc_LogLevel level, const char *message);

static inline OScInternal_LogDebug(OSc_Device *device, const char *message)
{
	OScInternal_Log(device, OSc_LogLevel_Debug, message);
}

static inline OScInternal_LogInfo(OSc_Device *device, const char *message)
{
	OScInternal_Log(device, OSc_LogLevel_Info, message);
}

static inline OScInternal_LogWarning(OSc_Device *device, const char *message)
{
	OScInternal_Log(device, OSc_LogLevel_Warning, message);
}

static inline OScInternal_LogError(OSc_Device *device, const char *message)
{
	OScInternal_Log(device, OSc_LogLevel_Error, message);
}

OScDev_PtrArray *OScInternal_PtrArray_Create(void);
void OScInternal_PtrArray_Destroy(const OScDev_PtrArray *arr);
void OScInternal_PtrArray_Append(OScDev_PtrArray *arr, void *obj);

OScDev_NumArray *OScInternal_NumArray_Create(void);
OScDev_NumArray *OScInternal_NumArray_Copy(const OScDev_NumArray *arr);
void OScInternal_NumArray_Destroy(const OScDev_NumArray *arr);
void OScInternal_NumArray_Append(OScDev_NumArray *arr, double val);
void OScInternal_NumArray_SortAscending(OScDev_NumArray *arr);
size_t OScInternal_NumArray_Size(OScDev_NumArray *arr);
double OScInternal_NumArray_At(OScDev_NumArray *arr, size_t index);
double OScInternal_NumArray_Min(const OScDev_NumArray *range);
double OScInternal_NumArray_Max(const OScDev_NumArray *range);

OScDev_NumRange *OScInternal_NumRange_CreateContinuous(double rMin, double rMax);
OScDev_NumRange *OScInternal_NumRange_CreateDiscrete(void);
void OScInternal_NumRange_Destroy(const OScDev_NumRange *range);
void OScInternal_NumRange_AppendDiscrete(OScDev_NumRange *range, double val);
bool OScInternal_NumRange_IsDiscrete(const OScDev_NumRange *range);
OScDev_NumArray *OScInternal_NumRange_DiscreteValues(const OScDev_NumRange *range);
double OScInternal_NumRange_Min(const OScDev_NumRange *range);
double OScInternal_NumRange_Max(const OScDev_NumRange *range);
double OScInternal_NumRange_ClosestValue(const OScDev_NumRange *range, double value);
OScDev_NumRange *OScInternal_NumRange_Intersection(
    const OScDev_NumRange *r1, const OScDev_NumRange *r2);
OScDev_NumRange *OScInternal_NumRange_Intersection3(
    const OScDev_NumRange *r1, const OScDev_NumRange *r2, const OScDev_NumRange *r3);
OScDev_NumRange *OScInternal_NumRange_Intersection4(
    const OScDev_NumRange *r1, const OScDev_NumRange *r2, const OScDev_NumRange *r3,
    const OScDev_NumRange *r4);

OSc_Error OScInternal_DeviceModule_GetCount(size_t *count);
OSc_Error OScInternal_DeviceModule_GetNames(const char **modules, size_t *count);
OSc_Error OScInternal_DeviceModule_GetDeviceImpls(const char *module, const OScDev_PtrArray **deviceImpls);

OSc_Error OScInternal_LSM_Associate_Device(OSc_LSM *lsm, OSc_Device *device);
OSc_Error OScInternal_LSM_Dissociate_Device(OSc_LSM *lsm, OSc_Device *device);
OSc_Error OScInternal_LSM_Is_Device_Associated(OSc_LSM *lsm, OSc_Device *device, bool *isAssociated);

OSc_Error OScInternal_Device_Create(OSc_Device **device, OScDev_DeviceImpl *impl, void *data);
OSc_Error OScInternal_Device_Destroy(OSc_Device *device);
bool OScInternal_Device_Log(OSc_Device *device, OSc_LogLevel level, const char *message);
void *OScInternal_Device_GetImplData(OSc_Device *device);
OScDev_NumRange *OScInternal_Device_GetPixelRates(OSc_Device *device);
OScDev_NumRange *OScInternal_Device_GetResolutions(OSc_Device *device);
OScDev_NumRange *OScInternal_Device_GetZooms(OSc_Device *device);
OSc_Error OScInternal_Device_GetNumberOfChannels(OSc_Device *device, uint32_t *numberOfChannels);
OSc_Error OScInternal_Device_GetBytesPerSample(OSc_Device *device, uint32_t *bytesPerSample);
OSc_Error OScInternal_Device_Arm(OSc_Device *device, OSc_Acquisition *acq);
OSc_Error OScInternal_Device_Start(OSc_Device *device);
void OScInternal_Device_Stop(OSc_Device *device);
void OScInternal_Device_Wait(OSc_Device *device);
OSc_Error OScInternal_Device_IsRunning(OSc_Device *device, bool *isRunning);

OSc_Error OScInternal_Setting_Create(OSc_Setting **setting, const char *name, OSc_ValueType valueType, OScDev_SettingImpl *impl, void *data);
void OScInternal_Setting_Destroy(OSc_Setting *setting);
void *OScInternal_Setting_GetImplData(OSc_Setting *setting);

OSc_Device *OScInternal_AcquisitionForDevice_GetDevice(OScDev_Acquisition *devAcq);
OSc_Acquisition *OScInternal_AcquisitionForDevice_GetAcquisition(OScDev_Acquisition *devAcq);

uint32_t OScInternal_Acquisition_GetNumberOfFrames(OSc_Acquisition *acq);
OSc_Device *OScInternal_Acquisition_GetClockDevice(OSc_Acquisition *acq);
OSc_Device *OScInternal_Acquisition_GetScannerDevice(OSc_Acquisition *acq);
OSc_Device *OScInternal_Acquisition_GetDetectorDevice(OSc_Acquisition *acq);
OScDev_Acquisition *OScInternal_Acquisition_GetForDevice(OSc_Acquisition *acq, OSc_Device *device);
bool OScInternal_Acquisition_CallFrameCallback(OSc_Acquisition *acq, uint32_t channel, void *pixels);
