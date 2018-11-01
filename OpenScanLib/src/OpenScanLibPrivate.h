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

OSc_Error OSc_LSM_Associate_Device(OSc_LSM *lsm, OSc_Device *device);
OSc_Error OSc_LSM_Dissociate_Device(OSc_LSM *lsm, OSc_Device *device);
OSc_Error OSc_LSM_Is_Device_Associated(OSc_LSM *lsm, OSc_Device *device, bool *isAssociated);

OSc_Error OSc_Device_Destroy(OSc_Device *device);