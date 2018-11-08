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


static OScDev_Error Acquisition_GetNumberOfFrames(struct OScDev_ModuleImpl *modImpl, OScDev_Acquisition *acq, uint32_t *numberOfFrames)
{
	*numberOfFrames = acq->numberOfFrames;
	return OScDev_OK;
}


static bool Acquisition_CallFrameCallback(struct OScDev_ModuleImpl *modImpl, OScDev_Acquisition *acq, uint32_t channel, void *pixels)
{
	return acq->frameCallback(acq, channel, pixels, acq->data);
}


struct OScDevInternal_Interface DeviceInterfaceFunctionTable = {
	.Log = Log,
	.Device_Create = Device_Create,
	.Device_GetImplData = Device_GetImplData,
	.Setting_Create = Setting_Create,
	.Setting_GetImplData = Setting_GetImplData,
	.Acquisition_GetNumberOfFrames = Acquisition_GetNumberOfFrames,
	.Acquisition_CallFrameCallback = Acquisition_CallFrameCallback,
};