#pragma once

#include "OpenScanLib.h"


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
	struct OSc_Device_Impl *impl;
	void *implData;

	OSc_Log_Func logFunc;
	void *logData;

	bool isOpen;

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
	OSc_Scanner *scanner;
	OSc_Detector *detector;

	OSc_Device **associatedDevices;
	size_t associatedDeviceCount;
};

struct OSc_Setting
{
	OSc_Device *device;
	OSc_Value_Type valueType;

	struct OSc_Setting_Impl *impl;
	void *implData;

	char name[OSc_MAX_STR_LEN + 1];
};

struct OSc_Acquisition
{
	OSc_Scanner *scanner;
	OSc_Detector *detector;
	uint32_t numberOfFrames;
	OSc_Trigger_Source triggerSource;
	OSc_Frame_Callback frameCallback;
	void *data;
};


// Internal functions

void OSc_Log(OSc_Device *device, OSc_Log_Level level, const char *message);

static inline void OSc_Log_Debug(OSc_Device *device, const char *message)
{
	OSc_Log(device, OSc_Log_Level_Debug, message);
}

static inline void OSc_Log_Info(OSc_Device *device, const char *message)
{
	OSc_Log(device, OSc_Log_Level_Info, message);
}

static inline void OSc_Log_Warning(OSc_Device *device, const char *message)
{
	OSc_Log(device, OSc_Log_Level_Warning, message);
}

static inline void OSc_Log_Error(OSc_Device *device, const char *message)
{
	OSc_Log(device, OSc_Log_Level_Error, message);
}

OSc_Error OSc_LSM_Associate_Device(OSc_LSM *lsm, OSc_Device *device);
OSc_Error OSc_LSM_Dissociate_Device(OSc_LSM *lsm, OSc_Device *device);
OSc_Error OSc_LSM_Is_Device_Associated(OSc_LSM *lsm, OSc_Device *device, bool *isAssociated);

OSc_Error OSc_API OSc_Device_Create(OSc_Device **device, struct OSc_Device_Impl *impl, void *data);
OSc_Error OSc_Device_Destroy(OSc_Device *device);

OSc_Error OSc_Setting_Create(OSc_Setting **setting, OSc_Device *device, const char *name, OSc_Value_Type valueType,
	struct OSc_Setting_Impl *impl, void *data);


OSc_Error OSc_Setting_NumericConstraintRange(OSc_Setting *setting, OSc_Value_Constraint *constraintType);
OSc_Error OSc_Setting_NumericConstraintDiscreteValues(OSc_Setting *setting, OSc_Value_Constraint *constraintType);
