#pragma once

#include "OpenScanLib.h"
#include "OpenScanDeviceLib.h"


// Internal functions

void OSc_Log(OSc_Device *device, OSc_LogLevel level, const char *message);

static inline OSc_Log_Debug(OSc_Device *device, const char *message)
{
	OSc_Log(device, OSc_LogLevel_Debug, message);
}

static inline OSc_Log_Info(OSc_Device *device, const char *message)
{
	OSc_Log(device, OSc_LogLevel_Info, message);
}

static inline OSc_Log_Warning(OSc_Device *device, const char *message)
{
	OSc_Log(device, OSc_LogLevel_Warning, message);
}

static inline OSc_Log_Error(OSc_Device *device, const char *message)
{
	OSc_Log(device, OSc_LogLevel_Error, message);
}

OScDev_PtrArray *OSc_PtrArray_Create(void);
void OSc_PtrArray_Destroy(const OScDev_PtrArray *arr);
void OSc_PtrArray_Append(OScDev_PtrArray *arr, void *obj);

OScDev_NumArray *OSc_NumArray_Create(void);
void OSc_NumArray_Destroy(const OScDev_NumArray *arr);
void OSc_NumArray_Append(OScDev_NumArray *arr, double val);

OScDev_NumRange *OSc_NumRange_CreateContinuous(double rMin, double rMax);
OScDev_NumRange *OSc_NumRange_CreateDiscrete(void);
void OSc_NumRange_Destroy(const OScDev_NumRange *range);
void OSc_NumRange_AppendDiscrete(OScDev_NumRange *range, double val);

OSc_Error OScInternal_LSM_Associate_Device(OSc_LSM *lsm, OSc_Device *device);
OSc_Error OScInternal_LSM_Dissociate_Device(OSc_LSM *lsm, OSc_Device *device);
OSc_Error OScInternal_LSM_Is_Device_Associated(OSc_LSM *lsm, OSc_Device *device, bool *isAssociated);

OSc_Error OScInternal_Device_Create(OSc_Device **device, OScDev_DeviceImpl *impl, void *data);
OSc_Error OScInternal_Device_Destroy(OSc_Device *device);
bool OScInternal_Device_Log(OSc_Device *device, OSc_LogLevel level, const char *message);
void *OScInternal_Device_GetImplData(OSc_Device *device);
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