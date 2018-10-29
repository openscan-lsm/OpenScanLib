#pragma once

#include "OpenScanLib.h"

#include <stdlib.h>


#define OSc_ENTRY_POINT_FUNC_NAME OpenScanDeviceModuleEntryPoint
#define OSc_ENTRY_POINT_FUNC __declspec(dllexport) OSc_ENTRY_POINT_FUNC_NAME


struct OSc_Device_Impl
{
	OSc_Error (*GetModelName)(const char **name);
	OSc_Error (*GetInstances)(OSc_Device ***devices, size_t *count);
	OSc_Error (*ReleaseInstance)(OSc_Device *device);

	OSc_Error (*GetName)(OSc_Device *device, char *name);
	OSc_Error (*Open)(OSc_Device *device);
	OSc_Error (*Close)(OSc_Device *device);

	OSc_Error (*HasScanner)(OSc_Device *device, bool *hasScanner);
	OSc_Error (*HasDetector)(OSc_Device *device, bool *hasDetector);

	OSc_Error (*GetSettings)(OSc_Device *device, OSc_Setting ***settings, size_t *count);

	OSc_Error (*GetAllowedResolutions)(OSc_Device *device, size_t **widths, size_t **heights, size_t *count);
	OSc_Error (*GetResolution)(OSc_Device *device, size_t *width, size_t *height);
	OSc_Error (*SetResolution)(OSc_Device *device, size_t width, size_t height);

	OSc_Error(*GetMagnification)(OSc_Device *device, double *magnification);
	OSc_Error(*SetMagnification)(OSc_Device *device);

	OSc_Error (*GetImageSize)(OSc_Device *device, uint32_t *width, uint32_t *height);
	OSc_Error (*GetNumberOfChannels)(OSc_Device *device, uint32_t *nChannels);
	OSc_Error (*GetBytesPerSample)(OSc_Device *device, uint32_t *bytesPerSample);

	OSc_Error (*ArmScanner)(OSc_Device *device, OSc_Acquisition *acq);
	OSc_Error (*StartScanner)(OSc_Device *device, OSc_Acquisition *acq);
	OSc_Error (*StopScanner)(OSc_Device *device, OSc_Acquisition *acq);

	OSc_Error (*ArmDetector)(OSc_Device *device, OSc_Acquisition *acq);
	OSc_Error (*StartDetector)(OSc_Device *device, OSc_Acquisition *acq);
	OSc_Error (*StopDetector)(OSc_Device *device, OSc_Acquisition *acq);

	OSc_Error (*IsRunning)(OSc_Device *device, bool *isRunning);
	OSc_Error (*Wait)(OSc_Device *device);
};


struct OSc_Setting_Impl
{
	OSc_Error (*IsEnabled)(OSc_Setting *setting, bool *enabled);
	OSc_Error (*IsWritable)(OSc_Setting *setting, bool *writable);
	OSc_Error (*GetNumericConstraintType)(OSc_Setting *setting, OSc_Value_Constraint *constraintType);
	OSc_Error (*GetString)(OSc_Setting *setting, char *value);
	OSc_Error (*SetString)(OSc_Setting *setting, const char *value);
	OSc_Error (*GetBool)(OSc_Setting *setting, bool *value);
	OSc_Error (*SetBool)(OSc_Setting *setting, bool value);
	OSc_Error (*GetInt32)(OSc_Setting *setting, int32_t *value);
	OSc_Error (*SetInt32)(OSc_Setting *setting, int32_t value);
	OSc_Error (*GetInt32Range)(OSc_Setting *setting, int32_t *min, int32_t *max);
	OSc_Error (*GetInt32DiscreteValues)(OSc_Setting *setting, int32_t **values, size_t *count);
	OSc_Error (*GetFloat64)(OSc_Setting *setting, double *value);
	OSc_Error (*SetFloat64)(OSc_Setting *setting, double value);
	OSc_Error (*GetFloat64Range)(OSc_Setting *setting, double *min, double *max);
	OSc_Error (*GetFloat64DiscreteValues)(OSc_Setting *setting, double **values, size_t *count);
	OSc_Error (*GetEnum)(OSc_Setting *setting, uint32_t *value);
	OSc_Error (*SetEnum)(OSc_Setting *setting, uint32_t value);
	OSc_Error (*GetEnumNumValues)(OSc_Setting *setting, uint32_t *count);
	OSc_Error (*GetEnumNameForValue)(OSc_Setting *setting, uint32_t value, char *name);
	OSc_Error (*GetEnumValueForName)(OSc_Setting *setting, uint32_t *value, const char *name);
};