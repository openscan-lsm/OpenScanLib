#include "OpenScanLibPrivate.h"
#include "InternalErrors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct OScInternal_Setting
{
	OScDev_ModuleImpl *modImpl;

	OScDev_SettingImpl *impl;
	// TODO It is such a common usage to set implData to the device instance,
	// that we should just provide a dedicated field for the device.
	void *implData;

	OSc_ValueType valueType;

	char name[OSc_MAX_STR_LEN + 1];

	OSc_SettingInvalidateFunc invalidateFunc;
	void *invalidateData;

	// Memoization should be avoided, but we need it for now to support the API
	// returning a static array.
	int32_t *i32DiscreteValues;
	size_t i32DiscreteValueCount;
	double *f64DiscreteValues;
	size_t f64DiscreteValueCount;
};


OSc_RichError *OSc_Setting_GetName(OSc_Setting *setting, char *name)
{
	strncpy(name, setting->name, OSc_MAX_STR_LEN);
	return OSc_Error_OK;
}


OSc_RichError *OSc_Setting_GetValueType(OSc_Setting *setting, OSc_ValueType *valueType)
{
	*valueType = setting->valueType;
	return OSc_Error_OK;
}


OSc_RichError *OSc_Setting_IsEnabled(OSc_Setting *setting, bool *enabled)
{
	OScDev_Error errCode = setting->impl->IsEnabled(setting, enabled);
	return OScInternal_Error_RetrieveFromSetting(setting, errCode);
}


OSc_RichError *OSc_Setting_IsWritable(OSc_Setting *setting, bool *writable)
{
	OScDev_Error errCode = setting->impl->IsWritable(setting, writable);
	return OScInternal_Error_RetrieveFromSetting(setting, errCode);
}


OSc_RichError *OSc_Setting_GetNumericConstraintType(OSc_Setting *setting, OSc_ValueConstraint *constraintType)
{
	*constraintType = OSc_ValueConstraint_None;
	OScDev_ValueConstraint dConstraintType;
	OScDev_Error errCode;
	errCode = setting->impl->GetNumericConstraintType(setting, &dConstraintType);
	if (errCode) {
		return OScInternal_Error_RetrieveFromSetting(setting, errCode);
	}
	*constraintType = (OSc_ValueConstraint)dConstraintType;
	return OSc_Error_OK;
}


void OSc_Setting_SetInvalidateCallback(OSc_Setting *setting, OSc_SettingInvalidateFunc func, void *data)
{
	if (!setting)
		return;
	setting->invalidateFunc = func;
	setting->invalidateData = data;
}


OSc_RichError *OSc_Setting_GetStringValue(OSc_Setting *setting, char *value)
{
	OScDev_Error errCode = setting->impl->GetString(setting, value);
	return OScInternal_Error_RetrieveFromSetting(setting, errCode);
}


OSc_RichError *OSc_Setting_SetStringValue(OSc_Setting *setting, const char *value)
{
	OScDev_Error errCode = setting->impl->SetString(setting, value);
	return OScInternal_Error_RetrieveFromSetting(setting, errCode);
}


OSc_RichError *OSc_Setting_GetBoolValue(OSc_Setting *setting, bool *value)
{
	OScDev_Error errCode = setting->impl->GetBool(setting, value);
	return OScInternal_Error_RetrieveFromSetting(setting, errCode);
}


OSc_RichError *OSc_Setting_SetBoolValue(OSc_Setting *setting, bool value)
{
	OScDev_Error errCode = setting->impl->SetBool(setting, value);
	return OScInternal_Error_RetrieveFromSetting(setting, errCode);
}


OSc_RichError *OSc_Setting_GetInt32Value(OSc_Setting *setting, int32_t *value)
{
	OScDev_Error errCode = setting->impl->GetInt32(setting, value);
	return OScInternal_Error_RetrieveFromSetting(setting, errCode);
}


OSc_RichError *OSc_Setting_SetInt32Value(OSc_Setting *setting, int32_t value)
{
	// TODO Should we validate the value here?
	OScDev_Error errCode = setting->impl->SetInt32(setting, value);
	return OScInternal_Error_RetrieveFromSetting(setting, errCode);
}


OSc_RichError *OSc_Setting_GetInt32ContinuousRange(OSc_Setting *setting, int32_t *min, int32_t *max)
{
	OScDev_Error errCode = setting->impl->GetInt32Range(setting, min, max);
	return OScInternal_Error_RetrieveFromSetting(setting, errCode);
}


OSc_RichError *OSc_Setting_GetInt32DiscreteValues(OSc_Setting *setting, int32_t **values, size_t *count)
{
	if (!setting || !count) {
		return OScInternal_Error_IllegalArgument();
	}

	if (setting->i32DiscreteValues == NULL) {
		OScDev_NumArray *values;
		OScDev_Error errCode;
		errCode = setting->impl->GetInt32DiscreteValues(setting, &values);
		if (errCode)
			return OScInternal_Error_RetrieveFromSetting(setting, errCode);

		if (values) {
			size_t count = OScInternal_NumArray_Size(values);
			setting->i32DiscreteValues = malloc(sizeof(int32_t) * count);
			for (size_t i = 0; i < count; ++i) {
				setting->i32DiscreteValues[i] = (int32_t)OScInternal_NumArray_At(values, i);
			}
			setting->i32DiscreteValueCount = count;
		}
		OScInternal_NumArray_Destroy(values);
	}

	*values = setting->i32DiscreteValues;
	*count = setting->i32DiscreteValueCount;
	return OSc_Error_OK;
}


OSc_RichError *OSc_Setting_GetFloat64Value(OSc_Setting *setting, double *value)
{
	OScDev_Error errCode = setting->impl->GetFloat64(setting, value);
	return OScInternal_Error_RetrieveFromSetting(setting, errCode);
}


OSc_RichError *OSc_Setting_SetFloat64Value(OSc_Setting *setting, double value)
{
	// TODO Should we validate the value here?
	OScDev_Error errCode = setting->impl->SetFloat64(setting, value);
	return OScInternal_Error_RetrieveFromSetting(setting, errCode);
}


OSc_RichError *OSc_Setting_GetFloat64ContinuousRange(OSc_Setting *setting, double *min, double *max)
{
	OScDev_Error errCode = setting->impl->GetFloat64Range(setting, min, max);
	return OScInternal_Error_RetrieveFromSetting(setting, errCode);
}


OSc_RichError *OSc_Setting_GetFloat64DiscreteValues(OSc_Setting *setting, double **values, size_t *count)
{
	if (!setting || !count) {
		return OScInternal_Error_IllegalArgument();
	}

	if (setting->f64DiscreteValues == NULL) {
		OScDev_NumArray *values;
		OScDev_Error errCode;
		errCode = setting->impl->GetFloat64DiscreteValues(setting, &values);
		if (errCode)
			return OScInternal_Error_RetrieveFromSetting(setting, errCode);

		if (values) {
			size_t count = OScInternal_NumArray_Size(values);
			setting->f64DiscreteValues = malloc(sizeof(double) * count);
			for (size_t i = 0; i < count; ++i) {
				setting->f64DiscreteValues[i] = OScInternal_NumArray_At(values, i);
			}
			setting->f64DiscreteValueCount = count;
		}
		OScInternal_NumArray_Destroy(values);
	}

	*values = setting->f64DiscreteValues;
	*count = setting->f64DiscreteValueCount;
	return OSc_Error_OK;
}


OSc_RichError *OSc_Setting_GetEnumValue(OSc_Setting *setting, uint32_t *value)
{
	OScDev_Error errCode = setting->impl->GetEnum(setting, value);
	return OScInternal_Error_RetrieveFromSetting(setting, errCode);
}


OSc_RichError *OSc_Setting_SetEnumValue(OSc_Setting *setting, uint32_t value)
{
	// TODO Should we validate the value here?
	OScDev_Error errCode = setting->impl->SetEnum(setting, value);
	return OScInternal_Error_RetrieveFromSetting(setting, errCode);
}


OSc_RichError *OSc_Setting_GetEnumNumValues(OSc_Setting *setting, uint32_t *count)
{
	OScDev_Error errCode = setting->impl->GetEnumNumValues(setting, count);
	return OScInternal_Error_RetrieveFromSetting(setting, errCode);
}


OSc_RichError *OSc_Setting_GetEnumNameForValue(OSc_Setting *setting, uint32_t value, char *name)
{
	OScDev_Error errCode = setting->impl->GetEnumNameForValue(setting, value, name);
	return OScInternal_Error_RetrieveFromSetting(setting, errCode);
}


OSc_RichError *OSc_Setting_GetEnumValueForName(OSc_Setting *setting, uint32_t *value, const char *name)
{
	OScDev_Error errCode = setting->impl->GetEnumValueForName(setting, value, name);
	return OScInternal_Error_RetrieveFromSetting(setting, errCode);
}


void *OScInternal_Setting_GetImplData(OSc_Setting *setting)
{
	return setting->implData;
}


void OScInternal_Setting_Invalidate(OSc_Setting *setting)
{
	if (setting->invalidateFunc)
		setting->invalidateFunc(setting, setting->invalidateData);
}


static OScDev_Error DefaultIsEnabled(OSc_Setting *setting, bool *enabled)
{
	*enabled = true;
	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultIsWritable(OSc_Setting *setting, bool *writable)
{
	*writable = true;
	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultGetNumericConstraint(OSc_Setting *setting, OScDev_ValueConstraint *constraintType)
{
	*constraintType = OScDev_ValueConstraint_None;
	switch (setting->valueType)
	{
	case OSc_ValueType_Int32:
	case OSc_ValueType_Float64:
		return OScInternal_LegacyError_OK;
	}
	return OScDev_Error_Wrong_Value_Type;
}


static OScDev_Error DefaultGetString(OSc_Setting *setting, char *value)
{
	strcpy(value, "");
	if (setting->valueType != OSc_ValueType_String)
		return OScDev_Error_Wrong_Value_Type;
	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultSetString(OSc_Setting *setting, const char *value)
{
	if (setting->valueType != OSc_ValueType_String)
		return OScDev_Error_Wrong_Value_Type;

	bool writable;
	OScDev_Error errCode;
	errCode = setting->impl->IsWritable(setting, &writable);
	if (errCode)
		return errCode;
	if (!writable)
		return OScDev_Error_Setting_Not_Writable;

	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultGetBool(OSc_Setting *setting, bool *value)
{
	*value = false;
	if (setting->valueType != OSc_ValueType_Bool)
		return OScDev_Error_Wrong_Value_Type;
	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultSetBool(OSc_Setting *setting, bool value)
{
	if (setting->valueType != OSc_ValueType_Bool)
		return OScDev_Error_Wrong_Value_Type;

	bool writable;
	OScDev_Error errCode;
	errCode = setting->impl->IsWritable(setting, &writable);
	if (errCode)
		return errCode;
	if (!writable)
		return OScDev_Error_Setting_Not_Writable;

	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultGetInt32(OSc_Setting *setting, int32_t *value)
{
	*value = 0;
	if (setting->valueType != OSc_ValueType_Int32)
		return OScDev_Error_Wrong_Value_Type;
	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultSetInt32(OSc_Setting *setting, int32_t value)
{
	if (setting->valueType != OSc_ValueType_Int32)
		return OScDev_Error_Wrong_Value_Type;

	bool writable;
	OScDev_Error errCode;
	errCode = setting->impl->IsWritable(setting, &writable);
	if (errCode)
		return errCode;
	if (!writable)
		return OScDev_Error_Setting_Not_Writable;

	// TODO Check constraint

	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultGetInt32Range(OSc_Setting *setting, int32_t *min, int32_t *max)
{
	*min = *max = 0;
	if (setting->valueType != OSc_ValueType_Int32)
		return OScDev_Error_Wrong_Value_Type;
	OSc_ValueConstraint constraint = OSc_ValueConstraint_None;
	OScDev_Error errCode;
	errCode = setting->impl->GetNumericConstraintType(setting, &constraint);
	if (errCode) {
		return errCode;
	}
	if (constraint != OSc_ValueConstraint_Continuous)
		return OScDev_Error_Wrong_Constraint_Type;
	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultGetInt32DiscreteValues(OSc_Setting *setting, OScDev_NumArray **values)
{
	*values = NULL;
	if (setting->valueType != OSc_ValueType_Int32)
		return OScDev_Error_Wrong_Value_Type;
	OSc_ValueConstraint constraint = OSc_ValueConstraint_None;
	OScDev_Error errCode;
	errCode = setting->impl->GetNumericConstraintType(setting, &constraint);
	if (errCode) {
		return errCode;
	}
	if (constraint != OSc_ValueConstraint_Discrete)
		return OScDev_Error_Wrong_Constraint_Type;
		
	*values = OScInternal_NumArray_Create();
	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultGetFloat64(OSc_Setting *setting, double *value)
{
	*value = 0.0;
	if (setting->valueType != OSc_ValueType_Float64)
		return OScDev_Error_Wrong_Value_Type;
	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultSetFloat64(OSc_Setting *setting, double value)
{
	if (setting->valueType != OSc_ValueType_Float64)
		return OScDev_Error_Wrong_Value_Type;

	bool writable;
	OScDev_Error errCode;
	errCode = setting->impl->IsWritable(setting, &writable);
	if (errCode)
		return errCode;
	if (!writable)
		return OScDev_Error_Setting_Not_Writable;

	// TODO Check constraint

	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultGetFloat64Range(OSc_Setting *setting, double *min, double *max)
{
	*min = *max = 0.0;
	if (setting->valueType != OSc_ValueType_Float64)
		return OScDev_Error_Wrong_Value_Type;
	OSc_ValueConstraint constraint = OSc_ValueConstraint_None;
	OScDev_Error errCode;
	errCode = setting->impl->GetNumericConstraintType(setting, &constraint);
	if (errCode) {
		return errCode;
	}
	if (constraint != OSc_ValueConstraint_Continuous)
		return OScDev_Error_Wrong_Constraint_Type;
	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultGetFloat64DiscreteValues(OSc_Setting *setting, OScDev_NumArray **values)
{
	*values = NULL;
	if (setting->valueType != OSc_ValueType_Float64)
		return OScDev_Error_Wrong_Value_Type;
	OSc_ValueConstraint constraint = OSc_ValueConstraint_None;
	OScDev_Error errCode;
	errCode = setting->impl->GetNumericConstraintType(setting, &constraint);
	if (errCode) {
		return errCode;
	}
	if (constraint != OSc_ValueConstraint_Discrete)
		return OScDev_Error_Wrong_Constraint_Type;
	*values = OScInternal_NumArray_Create();
	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultGetEnum(OSc_Setting *setting, uint32_t *value)
{
	*value = 0;
	if (setting->valueType != OSc_ValueType_Enum)
		return OScDev_Error_Wrong_Value_Type;
	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultSetEnum(OSc_Setting *setting, uint32_t value)
{
	if (setting->valueType != OSc_ValueType_Int32)
		return OScDev_Error_Wrong_Value_Type;

	bool writable;
	OScDev_Error errCode;
	errCode = setting->impl->IsWritable(setting, &writable);
	if (errCode)
		return errCode;
	if (!writable)
		return OScDev_Error_Setting_Not_Writable;

	// TODO Range Check

	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultGetEnumNumValues(OSc_Setting *setting, uint32_t *count)
{
	*count = 1;
	if (setting->valueType != OSc_ValueType_Enum)
		return OScDev_Error_Wrong_Value_Type;
	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultGetEnumNameForValue(OSc_Setting *setting, uint32_t value, char *name)
{
	strcpy(name, "");

	if (setting->valueType != OSc_ValueType_Enum)
		return OScDev_Error_Wrong_Value_Type;

	// TODO Check range

	snprintf(name, OSc_MAX_STR_LEN, "Value-%u", value);
	return OScInternal_LegacyError_OK;
}


static OScDev_Error DefaultGetEnumValueForName(OSc_Setting *setting, uint32_t *value, const char *name)
{
	value = 0;

	if (setting->valueType != OSc_ValueType_Enum)
		return OScDev_Error_Wrong_Value_Type;

	if (strstr(name, "Value-") != 0)
		return OScDev_Error_Unknown_Enum_Value_Name;
	long parsedNum = atol(name + strlen("Value-"));

	// TODO Check range

	*value = parsedNum;
	return OScInternal_LegacyError_OK;
}


static void DefaultRelease(OSc_Setting *setting)
{
}


OSc_RichError *OScInternal_Setting_Create(OScDev_ModuleImpl *modImpl, OSc_Setting **setting, const char *name, OSc_ValueType valueType,
	OScDev_SettingImpl *impl, void *data)
{
	// TODO We should not modify 'impl' which belongs to the device module.
	// Instead we should either use a copy of 'impl' or just check for NULL
	// on each call to an impl function.
	if (impl->IsEnabled == NULL)
		impl->IsEnabled = DefaultIsEnabled;
	if (impl->IsWritable == NULL)
		impl->IsWritable = DefaultIsWritable;
	if (impl->GetNumericConstraintType == NULL)
		impl->GetNumericConstraintType = DefaultGetNumericConstraint;
	if (impl->GetString == NULL)
		impl->GetString = DefaultGetString;
	if (impl->SetString == NULL)
		impl->SetString = DefaultSetString;
	if (impl->GetBool == NULL)
		impl->GetBool = DefaultGetBool;
	if (impl->GetInt32 == NULL)
		impl->GetInt32 = DefaultGetInt32;
	if (impl->SetInt32 == NULL)
		impl->SetInt32 = DefaultSetInt32;
	if (impl->GetInt32Range == NULL)
		impl->GetInt32Range = DefaultGetInt32Range;
	if (impl->GetInt32DiscreteValues == NULL)
		impl->GetInt32DiscreteValues = DefaultGetInt32DiscreteValues;
	if (impl->GetFloat64 == NULL)
		impl->GetFloat64 = DefaultGetFloat64;
	if (impl->SetFloat64 == NULL)
		impl->SetFloat64 = DefaultSetFloat64;
	if (impl->GetFloat64Range == NULL)
		impl->GetFloat64Range = DefaultGetFloat64Range;
	if (impl->GetFloat64DiscreteValues == NULL)
		impl->GetFloat64DiscreteValues = DefaultGetFloat64DiscreteValues;
	if (impl->GetEnum == NULL)
		impl->GetEnum = DefaultGetEnum;
	if (impl->SetEnum == NULL)
		impl->SetEnum = DefaultSetEnum;
	if (impl->GetEnumNumValues == NULL)
		impl->GetEnumNumValues = DefaultGetEnumNumValues;
	if (impl->GetEnumNameForValue == NULL)
		impl->GetEnumNameForValue = DefaultGetEnumNameForValue;
	if (impl->GetEnumValueForName == NULL)
		impl->GetEnumValueForName = DefaultGetEnumValueForName;
	if (impl->Release == NULL)
		impl->Release = DefaultRelease;

	*setting = calloc(1, sizeof(OSc_Setting));
	(*setting)->modImpl = modImpl;
	(*setting)->impl = impl;
	(*setting)->implData = data;
	(*setting)->valueType = valueType;
	strncpy((*setting)->name, name, OSc_MAX_STR_LEN);
	return OSc_Error_OK;
}


void OScInternal_Setting_Destroy(OSc_Setting *setting)
{
	setting->impl->Release(setting);
	free(setting);
}


OSc_RichError *OSc_Setting_NumericConstraintRange(OSc_Setting *setting, OSc_ValueConstraint *constraintType)
{
	*constraintType = OSc_ValueConstraint_Continuous;
	return OSc_Error_OK;
}


OSc_RichError *OSc_Setting_NumericConstraintDiscreteValues(OSc_Setting *setting, OSc_ValueConstraint *constraintType)
{
	*constraintType = OSc_ValueConstraint_Discrete;
	return OSc_Error_OK;
}


void *OSc_Setting_GetImplData(OSc_Setting *setting)
{
	return setting->implData;
}


bool OScInternal_Setting_SupportsRichErrors(OSc_Setting *setting)
{
	// settings that are attached to AcqTemplate
	if (!setting->modImpl)
		return true;
	return setting->modImpl->supportsRichErrors;
}
