#include "OpenScanLibPrivate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


OSc_Error OSc_API OSc_Setting_GetName(OSc_Setting *setting, char *name)
{
	strncpy(name, setting->name, OSc_MAX_STR_LEN);
	return OSc_Error_OK;
}


OSc_Error OSc_Setting_GetValueType(OSc_Setting *setting, OSc_ValueType *valueType)
{
	*valueType = setting->valueType;
	return OSc_Error_OK;
}


OSc_Error OSc_Setting_IsEnabled(OSc_Setting *setting, bool *enabled)
{
	return setting->impl->IsEnabled(setting, enabled);
}


OSc_Error OSc_Setting_IsWritable(OSc_Setting *setting, bool *writable)
{
	return setting->impl->IsWritable(setting, writable);
}


OSc_Error OSc_Setting_GetNumericConstraintType(OSc_Setting *setting, OSc_ValueConstraint *constraintType)
{
	*constraintType = OSc_ValueConstraint_None;
	enum OScDev_ValueConstraint dConstraintType;
	OScDev_Error err;
	if (OSc_CHECK_ERROR(err, setting->impl->GetNumericConstraintType(setting, &dConstraintType)))
		return err;
	*constraintType = (OSc_ValueConstraint)dConstraintType;
	return OSc_Error_OK;
}


OSc_Error OSc_Setting_GetStringValue(OSc_Setting *setting, char *value)
{
	return setting->impl->GetString(setting, value);
}


OSc_Error OSc_Setting_SetStringValue(OSc_Setting *setting, const char *value)
{
	return setting->impl->SetString(setting, value);
}


OSc_Error OSc_Setting_GetBoolValue(OSc_Setting *setting, bool *value)
{
	return setting->impl->GetBool(setting, value);
}


OSc_Error OSc_Setting_SetBoolValue(OSc_Setting *setting, bool value)
{
	return setting->impl->SetBool(setting, value);
}


OSc_Error OSc_Setting_GetInt32Value(OSc_Setting *setting, int32_t *value)
{
	return setting->impl->GetInt32(setting, value);
}


OSc_Error OSc_Setting_SetInt32Value(OSc_Setting *setting, int32_t value)
{
	return setting->impl->SetInt32(setting, value);
}


OSc_Error OSc_Setting_GetInt32ContinuousRange(OSc_Setting *setting, int32_t *min, int32_t *max)
{
	return setting->impl->GetInt32Range(setting, min, max);
}


OSc_Error OSc_Setting_GetInt32DiscreteValues(OSc_Setting *setting, int32_t **values, size_t *count)
{
	return setting->impl->GetInt32DiscreteValues(setting, values, count);
}


OSc_Error OSc_Setting_GetFloat64Value(OSc_Setting *setting, double *value)
{
	return setting->impl->GetFloat64(setting, value);
}


OSc_Error OSc_Setting_SetFloat64Value(OSc_Setting *setting, double value)
{
	return setting->impl->SetFloat64(setting, value);
}


OSc_Error OSc_Setting_GetFloat64ContinuousRange(OSc_Setting *setting, double *min, double *max)
{
	return setting->impl->GetFloat64Range(setting, min, max);
}


OSc_Error OSc_Setting_GetFloat64DiscreteValues(OSc_Setting *setting, double **values, size_t *count)
{
	return setting->impl->GetFloat64DiscreteValues(setting, values, count);
}


OSc_Error OSc_Setting_GetEnumValue(OSc_Setting *setting, uint32_t *value)
{
	return setting->impl->GetEnum(setting, value);
}


OSc_Error OSc_Setting_SetEnumValue(OSc_Setting *setting, uint32_t value)
{
	return setting->impl->SetEnum(setting, value);
}


OSc_Error OSc_Setting_GetEnumNumValues(OSc_Setting *setting, uint32_t *count)
{
	return setting->impl->GetEnumNumValues(setting, count);
}


OSc_Error OSc_Setting_GetEnumNameForValue(OSc_Setting *setting, uint32_t value, char *name)
{
	return setting->impl->GetEnumNameForValue(setting, value, name);
}


OSc_Error OSc_Setting_GetEnumValueForName(OSc_Setting *setting, uint32_t *value, const char *name)
{
	return setting->impl->GetEnumValueForName(setting, value, name);
}


static OSc_Error DefaultIsEnabled(OSc_Setting *setting, bool *enabled)
{
	*enabled = true;
	return OSc_Error_OK;
}


static OSc_Error DefaultIsWritable(OSc_Setting *setting, bool *writable)
{
	*writable = true;
	return OSc_Error_OK;
}


static OSc_Error DefaultGetNumericConstraint(OSc_Setting *setting, enum OScDev_ValueConstraint *constraintType)
{
	*constraintType = OScDev_ValueConstraint_None;
	switch (setting->valueType)
	{
	case OSc_ValueType_Int32:
	case OSc_ValueType_Float64:
		return OSc_Error_OK;
	}
	return OSc_Error_Wrong_Value_Type;
}


static OSc_Error DefaultGetString(OSc_Setting *setting, char *value)
{
	strcpy(value, "");
	if (setting->valueType != OSc_ValueType_String)
		return OSc_Error_Wrong_Value_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultSetString(OSc_Setting *setting, const char *value)
{
	if (setting->valueType != OSc_ValueType_String)
		return OSc_Error_Wrong_Value_Type;

	bool writable;
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, OSc_Setting_IsWritable(setting, &writable)))
		return err;
	if (!writable)
		return OSc_Error_Setting_Not_Writable;

	return OSc_Error_OK;
}


static OSc_Error DefaultGetBool(OSc_Setting *setting, bool *value)
{
	*value = false;
	if (setting->valueType != OSc_ValueType_Bool)
		return OSc_Error_Wrong_Value_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultSetBool(OSc_Setting *setting, bool value)
{
	if (setting->valueType != OSc_ValueType_Bool)
		return OSc_Error_Wrong_Value_Type;

	bool writable;
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, OSc_Setting_IsWritable(setting, &writable)))
		return err;
	if (!writable)
		return OSc_Error_Setting_Not_Writable;

	return OSc_Error_OK;
}


static OSc_Error DefaultGetInt32(OSc_Setting *setting, int32_t *value)
{
	*value = 0;
	if (setting->valueType != OSc_ValueType_Int32)
		return OSc_Error_Wrong_Value_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultSetInt32(OSc_Setting *setting, int32_t value)
{
	if (setting->valueType != OSc_ValueType_Int32)
		return OSc_Error_Wrong_Value_Type;

	bool writable;
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, OSc_Setting_IsWritable(setting, &writable)))
		return err;
	if (!writable)
		return OSc_Error_Setting_Not_Writable;

	// TODO Check constraint

	return OSc_Error_OK;
}


static OSc_Error DefaultGetInt32Range(OSc_Setting *setting, int32_t *min, int32_t *max)
{
	*min = *max = 0;
	if (setting->valueType != OSc_ValueType_Int32)
		return OSc_Error_Wrong_Value_Type;
	OSc_ValueConstraint constraint;
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, OSc_Setting_GetNumericConstraintType(setting, &constraint)))
		return err;
	if (constraint != OSc_ValueConstraint_Continuous)
		return OSc_Error_Wrong_Constraint_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultGetInt32DiscreteValues(OSc_Setting *setting, int32_t **values, size_t *count)
{
	static int32_t v = { 0 };
	*values = &v;
	*count = 1;
	if (setting->valueType != OSc_ValueType_Int32)
		return OSc_Error_Wrong_Value_Type;
	OSc_ValueConstraint constraint;
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, OSc_Setting_GetNumericConstraintType(setting, &constraint)))
		return err;
	if (constraint != OSc_ValueConstraint_Discrete)
		return OSc_Error_Wrong_Constraint_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultGetFloat64(OSc_Setting *setting, double *value)
{
	*value = 0.0;
	if (setting->valueType != OSc_ValueType_Float64)
		return OSc_Error_Wrong_Value_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultSetFloat64(OSc_Setting *setting, double value)
{
	if (setting->valueType != OSc_ValueType_Float64)
		return OSc_Error_Wrong_Value_Type;

	bool writable;
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, OSc_Setting_IsWritable(setting, &writable)))
		return err;
	if (!writable)
		return OSc_Error_Setting_Not_Writable;

	// TODO Check constraint

	return OSc_Error_OK;
}


static OSc_Error DefaultGetFloat64Range(OSc_Setting *setting, double *min, double *max)
{
	*min = *max = 0.0;
	if (setting->valueType != OSc_ValueType_Float64)
		return OSc_Error_Wrong_Value_Type;
	OSc_ValueConstraint constraint;
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, OSc_Setting_GetNumericConstraintType(setting, &constraint)))
		return err;
	if (constraint != OSc_ValueConstraint_Continuous)
		return OSc_Error_Wrong_Constraint_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultGetFloat64DiscreteValues(OSc_Setting *setting, double **values, size_t *count)
{
	static double v = { 0.0 };
	*values = &v;
	*count = 1;
	if (setting->valueType != OSc_ValueType_Float64)
		return OSc_Error_Wrong_Value_Type;
	OSc_ValueConstraint constraint;
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, OSc_Setting_GetNumericConstraintType(setting, &constraint)))
		return err;
	if (constraint != OSc_ValueConstraint_Discrete)
		return OSc_Error_Wrong_Constraint_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultGetEnum(OSc_Setting *setting, uint32_t *value)
{
	*value = 0;
	if (setting->valueType != OSc_ValueType_Enum)
		return OSc_Error_Wrong_Value_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultSetEnum(OSc_Setting *setting, uint32_t value)
{
	if (setting->valueType != OSc_ValueType_Int32)
		return OSc_Error_Wrong_Value_Type;

	bool writable;
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, OSc_Setting_IsWritable(setting, &writable)))
		return err;
	if (!writable)
		return OSc_Error_Setting_Not_Writable;

	// TODO Range Check

	return OSc_Error_OK;
}


static OSc_Error DefaultGetEnumNumValues(OSc_Setting *setting, uint32_t *count)
{
	*count = 1;
	if (setting->valueType != OSc_ValueType_Enum)
		return OSc_Error_Wrong_Value_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultGetEnumNameForValue(OSc_Setting *setting, uint32_t value, char *name)
{
	strcpy(name, "");

	if (setting->valueType != OSc_ValueType_Enum)
		return OSc_Error_Wrong_Value_Type;

	// TODO Check range

	snprintf(name, OSc_MAX_STR_LEN, "Value-%u", value);
	return OSc_Error_OK;
}


static OSc_Error DefaultGetEnumValueForName(OSc_Setting *setting, uint32_t *value, const char *name)
{
	value = 0;

	if (setting->valueType != OSc_ValueType_Enum)
		return OSc_Error_Wrong_Value_Type;

	if (strstr(name, "Value-") != 0)
		return OSc_Error_Unknown_Enum_Value_Name;
	long parsedNum = atol(name + strlen("Value-"));

	// TODO Check range

	*value = parsedNum;
	return OSc_Error_OK;
}


static void DefaultRelease(OSc_Setting *setting)
{
}


OSc_Error OSc_Setting_Create(OSc_Setting **setting, const char *name, OSc_ValueType valueType,
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
	(*setting)->impl = impl;
	(*setting)->implData = data;
	(*setting)->valueType = valueType;
	strncpy((*setting)->name, name, OSc_MAX_STR_LEN);
	return OSc_Error_OK;
}


void OSc_Setting_Destroy(OSc_Setting *setting)
{
	setting->impl->Release(setting);
	free(setting);
}


OSc_Error OSc_Setting_NumericConstraintRange(OSc_Setting *setting, OSc_ValueConstraint *constraintType)
{
	*constraintType = OSc_ValueConstraint_Continuous;
	return OSc_Error_OK;
}


OSc_Error OSc_Setting_NumericConstraintDiscreteValues(OSc_Setting *setting, OSc_ValueConstraint *constraintType)
{
	*constraintType = OSc_ValueConstraint_Discrete;
	return OSc_Error_OK;
}


void *OSc_Setting_GetImplData(OSc_Setting *setting)
{
	return setting->implData;
}