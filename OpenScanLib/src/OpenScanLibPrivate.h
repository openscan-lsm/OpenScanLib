#pragma once

#include "OpenScanLib.h"
#include "OpenScanDeviceLib.h"


struct OSc_Clock
{
	OSc_Device *device;
};

struct OSc_Scanner
{
	OSc_Device *device;
};

struct OSc_Detector
{
	OSc_Device *device;
};

struct OSc_Device
{
	struct OScDev_DeviceImpl *impl;
	void *implData;

	OSc_Log_Func logFunc;
	void *logData;

	bool isOpen;

	OSc_Clock *clock;
	OSc_Scanner *scanner;
	OSc_Detector *detector;

	OSc_LSM *associatedLSM;

	OSc_Setting **settings;
	size_t numSettings;

	char name[OSc_MAX_STR_LEN + 1];
	char displayName[OSc_MAX_STR_LEN + 1];
};

struct OSc_LSM
{
	OSc_Clock *clock;
	OSc_Scanner *scanner;
	OSc_Detector *detector;

	OSc_Device **associatedDevices;
	size_t associatedDeviceCount;
};

struct OSc_Setting
{
	struct OScDev_SettingImpl *impl;
	void *implData;

	OSc_Value_Type valueType;

	char name[OSc_MAX_STR_LEN + 1];
};

struct OSc_AcquisitionForDevice
{
	OSc_Device *device;
	OSc_Acquisition *acq;
};

struct OSc_Acquisition
{
	OSc_Clock *clock;
	OSc_Scanner *scanner;
	OSc_Detector *detector;
	uint32_t numberOfFrames;
	OSc_Frame_Callback frameCallback;
	void *data;

	// We can pass opaque pointers to these structs to devices, so that we can
	// handle acquisition-related calls in a device-specific manner.
	struct OSc_AcquisitionForDevice acqForClockDevice;
	struct OSc_AcquisitionForDevice acqForScannerDevice;
	struct OSc_AcquisitionForDevice acqForDetectorDevice;
};


// Internal functions

void OSc_Log(OSc_Device *device, OSc_Log_Level level, const char *message);

static inline OSc_Log_Debug(OSc_Device *device, const char *message)
{
	OSc_Log(device, OSc_Log_Level_Debug, message);
}

static inline OSc_Log_Info(OSc_Device *device, const char *message)
{
	OSc_Log(device, OSc_Log_Level_Info, message);
}

static inline OSc_Log_Warning(OSc_Device *device, const char *message)
{
	OSc_Log(device, OSc_Log_Level_Warning, message);
}

static inline OSc_Log_Error(OSc_Device *device, const char *message)
{
	OSc_Log(device, OSc_Log_Level_Error, message);
}

OScDev_PtrArray *OSc_PtrArray_Create(void);
void OSc_PtrArray_Destroy(const OScDev_PtrArray *arr);
void OSc_PtrArray_Append(OScDev_PtrArray *arr, void *obj);

OSc_Error OSc_LSM_Associate_Device(OSc_LSM *lsm, OSc_Device *device);
OSc_Error OSc_LSM_Dissociate_Device(OSc_LSM *lsm, OSc_Device *device);
OSc_Error OSc_LSM_Is_Device_Associated(OSc_LSM *lsm, OSc_Device *device, bool *isAssociated);

OSc_Error OSc_Device_Create(OSc_Device **device, struct OScDev_DeviceImpl *impl, void *data);
OSc_Error OSc_Device_Destroy(OSc_Device *device);

OSc_Error OSc_Setting_Create(OSc_Setting **setting, const char *name, OSc_Value_Type valueType, struct OScDev_SettingImpl *impl, void *data);
