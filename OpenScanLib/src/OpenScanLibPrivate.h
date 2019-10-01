#pragma once

#include "OpenScanLib.h"
#include "OpenScanDeviceLib.h"


struct OScInternal_Clock
{
	OSc_Device *device;
};

struct OScInternal_Scanner
{
	OSc_Device *device;
};

struct OScInternal_Detector
{
	OSc_Device *device;
};

struct OScInternal_Device
{
	OScDev_DeviceImpl *impl;
	void *implData;

	OSc_LogFunc logFunc;
	void *logData;

	bool isOpen;

	OSc_Clock *clock;
	OSc_Scanner *scanner;
	OSc_Detector *detector;

	OSc_LSM *associatedLSM;

	OScDev_PtrArray *settings;

	char name[OSc_MAX_STR_LEN + 1];
	char displayName[OSc_MAX_STR_LEN + 1];
};

struct OScInternal_LSM
{
	OSc_Clock *clock;
	OSc_Scanner *scanner;
	OSc_Detector *detector;

	OSc_Device **associatedDevices;
	size_t associatedDeviceCount;
};

struct OScInternal_Setting
{
	OScDev_SettingImpl *impl;
	// TODO It is such a common usage to set implData to the device instance,
	// that we should just provide a dedicated field for the device.
	void *implData;

	OSc_ValueType valueType;

	char name[OSc_MAX_STR_LEN + 1];
};

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

OSc_Error OSc_LSM_Associate_Device(OSc_LSM *lsm, OSc_Device *device);
OSc_Error OSc_LSM_Dissociate_Device(OSc_LSM *lsm, OSc_Device *device);
OSc_Error OSc_LSM_Is_Device_Associated(OSc_LSM *lsm, OSc_Device *device, bool *isAssociated);

OSc_Error OSc_Device_Create(OSc_Device **device, OScDev_DeviceImpl *impl, void *data);
OSc_Error OSc_Device_Destroy(OSc_Device *device);

OSc_Error OSc_Setting_Create(OSc_Setting **setting, const char *name, OSc_ValueType valueType, OScDev_SettingImpl *impl, void *data);
void OSc_Setting_Destroy(OSc_Setting *setting);