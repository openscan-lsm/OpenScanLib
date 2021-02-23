#pragma once

#define OScDevInternal_BUILDING_OPENSCANLIB 1
#include "OpenScanLib.h"
#include "OpenScanDeviceLib.h"

#include <RichErrors/Err2Code.h>


typedef struct RERR_Error OScInternal_RichError;
#define OScInternal_LegacyError_OK (OScDev_Error)0

// APIs for OpenScanLib
OSc_RichError *OScInternal_Error_RetrieveFromDevice(OSc_Device *device, int32_t code);

OSc_RichError *OScInternal_Error_RetrieveFromSetting(OSc_Setting *setting, int32_t code);

OSc_RichError *OScInternal_Error_RetrieveFromModule(OScDev_ModuleImpl *modImpl, int32_t code);

OSc_RichError *OScInternal_Error_Create(const char *message);

OSc_RichError *OScInternal_Error_CreateWithCode(const char *domainName, int32_t code, const char *message);

OSc_RichError *OScInternal_Error_Wrap(OSc_RichError *cause, const char *message);

OSc_RichError *OScInternal_Error_WrapWithCode(OSc_RichError *cause, const char *domainName, int32_t code, const char *message);

OScDev_Error OScInternal_Error_ReturnAsCode(OScDev_RichError *error);

OScDev_RichError *OScInternal_Error_RegisterCodeDomain(const char *domainName, RERR_CodeFormat codeFormat);

char *OScInternal_Error_LegacyCodeDomain();

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


typedef struct OScInternal_PtrArray OScInternal_PtrArray;
typedef struct OScInternal_NumArray OScInternal_NumArray;
typedef struct OScInternal_NumRange OScInternal_NumRange;


OScInternal_PtrArray *OScInternal_PtrArray_Create(void);
OScInternal_PtrArray *OScInternal_PtrArray_CreateFromNullTerminated(void *const *nullTerminatedArray);
void OScInternal_PtrArray_Destroy(const OScInternal_PtrArray *arr);
void OScInternal_PtrArray_Append(OScInternal_PtrArray *arr, void *obj);
size_t OScInternal_PtrArray_Size(const OScInternal_PtrArray *arr);
bool OScInternal_PtrArray_Empty(const OScInternal_PtrArray *arr);
void *OScInternal_PtrArray_At(const OScInternal_PtrArray *arr, size_t index);
void *const *OScInternal_PtrArray_Data(const OScInternal_PtrArray *arr);

OScInternal_NumArray *OScInternal_NumArray_Create(void);
OScInternal_NumArray *OScInternal_NumArray_CreateFromNaNTerminated(const double *nanTerminatedArray);
OScInternal_NumArray *OScInternal_NumArray_Copy(const OScInternal_NumArray *arr);
void OScInternal_NumArray_Destroy(const OScInternal_NumArray *arr);
void OScInternal_NumArray_Append(OScInternal_NumArray *arr, double val);
void OScInternal_NumArray_SortAscending(OScInternal_NumArray *arr);
size_t OScInternal_NumArray_Size(const OScInternal_NumArray *arr);
bool OScInternal_NumArray_Empty(const OScInternal_NumArray *arr);
double OScInternal_NumArray_At(const OScInternal_NumArray *arr, size_t index);
double OScInternal_NumArray_Min(const OScInternal_NumArray *range);
double OScInternal_NumArray_Max(const OScInternal_NumArray *range);

OScInternal_NumRange *OScInternal_NumRange_CreateContinuous(double rMin, double rMax);
OScInternal_NumRange *OScInternal_NumRange_CreateDiscrete(void);
OScInternal_NumRange *OScInternal_NumRange_CreateDiscreteFromNaNTerminated(const double *nanTerminatedArray);
void OScInternal_NumRange_Destroy(const OScInternal_NumRange *range);
void OScInternal_NumRange_AppendDiscrete(OScInternal_NumRange *range, double val);
bool OScInternal_NumRange_IsDiscrete(const OScInternal_NumRange *range);
OScInternal_NumArray *OScInternal_NumRange_DiscreteValues(const OScInternal_NumRange *range);
double OScInternal_NumRange_Min(const OScInternal_NumRange *range);
double OScInternal_NumRange_Max(const OScInternal_NumRange *range);
double OScInternal_NumRange_ClosestValue(const OScInternal_NumRange *range, double value);
bool OScInternal_NumRange_Contains(const OScInternal_NumRange *range, double value);
OScInternal_NumRange *OScInternal_NumRange_Intersection(
	const OScInternal_NumRange *r1, const OScInternal_NumRange *r2);
OScInternal_NumRange *OScInternal_NumRange_Intersection3(
	const OScInternal_NumRange *r1, const OScInternal_NumRange *r2, const OScInternal_NumRange *r3);
OScInternal_NumRange *OScInternal_NumRange_Intersection4(
	const OScInternal_NumRange *r1, const OScInternal_NumRange *r2, const OScInternal_NumRange *r3,
	const OScInternal_NumRange *r4);
OScInternal_NumRange *OScInternal_NumRange_Intersection5(
	const OScInternal_NumRange *r1, const OScInternal_NumRange *r2, const OScInternal_NumRange *r3,
	const OScInternal_NumRange *r4, const OScInternal_NumRange *r5);
OScInternal_NumRange *OScInternal_NumRange_Intersection6(
	const OScInternal_NumRange *r1, const OScInternal_NumRange *r2, const OScInternal_NumRange *r3,
	const OScInternal_NumRange *r4, const OScInternal_NumRange *r5, const OScInternal_NumRange *r6);

OSc_RichError *OScInternal_DeviceModule_GetCount(size_t *count);
OSc_RichError *OScInternal_DeviceModule_GetNames(const char **modules, size_t *count);
OSc_RichError *OScInternal_DeviceModule_GetDeviceImpls(const char *module, OScInternal_PtrArray **deviceImpls);

OSc_RichError *OScInternal_LSM_Associate_Device(OSc_LSM *lsm, OSc_Device *device);
OSc_RichError *OScInternal_LSM_Dissociate_Device(OSc_LSM *lsm, OSc_Device *device);
OSc_RichError *OScInternal_LSM_Is_Device_Associated(OSc_LSM *lsm, OSc_Device *device, bool *isAssociated);

OScDev_Error OScInternal_Device_Create(OScDev_ModuleImpl *modImpl, OSc_Device **device, OScDev_DeviceImpl *impl, void *data);
OSc_RichError *OScInternal_Device_Destroy(OSc_Device *device);
bool OScInternal_Device_Log(OSc_Device *device, OSc_LogLevel level, const char *message);
void *OScInternal_Device_GetImplData(OSc_Device *device);
OScInternal_NumRange *OScInternal_Device_GetPixelRates(OSc_Device *device);
OScInternal_NumRange *OScInternal_Device_GetResolutions(OSc_Device *device);
OScInternal_NumRange *OScInternal_Device_GetZooms(OSc_Device *device);
bool OScInternal_Device_IsROIScanSupported(OSc_Device *device);
OScInternal_NumRange *OScInternal_Device_GetRasterWidths(OSc_Device *device);
OScInternal_NumRange *OScInternal_Device_GetRasterHeights(OSc_Device *device);
OSc_RichError *OScInternal_Device_GetNumberOfChannels(OSc_Device *device, uint32_t *numberOfChannels);
OSc_RichError *OScInternal_Device_GetBytesPerSample(OSc_Device *device, uint32_t *bytesPerSample);
OSc_RichError *OScInternal_Device_Arm(OSc_Device *device, OSc_Acquisition *acq);
OSc_RichError *OScInternal_Device_Start(OSc_Device *device);
void OScInternal_Device_Stop(OSc_Device *device);
void OScInternal_Device_Wait(OSc_Device *device);
OSc_RichError *OScInternal_Device_IsRunning(OSc_Device *device, bool *isRunning);
bool OScInternal_Device_SupportsRichErrors(OSc_Device *device);

OSc_RichError *OScInternal_Setting_Create(OScDev_ModuleImpl *modImpl, OSc_Setting **setting, const char *name, OSc_ValueType valueType, OScDev_SettingImpl *impl, void *data);
void OScInternal_Setting_Destroy(OSc_Setting *setting);
void *OScInternal_Setting_GetImplData(OSc_Setting *setting);
void OScInternal_Setting_Invalidate(OSc_Setting *setting);
bool OScInternal_Setting_SupportsRichErrors(OSc_Setting *setting);

bool OScInternal_Module_SupportsRichErrors(OScDev_ModuleImpl *modImpl);

OSc_Device *OScInternal_AcquisitionForDevice_GetDevice(OScDev_Acquisition *devAcq);
OSc_Acquisition *OScInternal_AcquisitionForDevice_GetAcquisition(OScDev_Acquisition *devAcq);

uint32_t OScInternal_Acquisition_GetNumberOfFrames(OSc_Acquisition *acq);
OSc_Device *OScInternal_Acquisition_GetClockDevice(OSc_Acquisition *acq);
OSc_Device *OScInternal_Acquisition_GetScannerDevice(OSc_Acquisition *acq);
OSc_Device *OScInternal_Acquisition_GetDetectorDevice(OSc_Acquisition *acq);
OScDev_Acquisition *OScInternal_Acquisition_GetForDevice(OSc_Acquisition *acq, OSc_Device *device);
bool OScInternal_Acquisition_CallFrameCallback(OSc_Acquisition *acq, uint32_t channel, void *pixels);
