#include "OpenScanLibPrivate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


OSc_Error OSc_API OSc_Setting_Get_Name(OSc_Setting *setting, char *name)
{
	strncpy(name, setting->name, OSc_MAX_STR_LEN);
	return OSc_Error_OK;
}


OSc_Error OSc_Setting_Get_Value_Type(OSc_Setting *setting, OSc_Value_Type *valueType)
{
	*valueType = setting->valueType;
	return OSc_Error_OK;
}


OSc_Error OSc_Setting_Is_Enabled(OSc_Setting *setting, bool *enabled)
{
	return setting->impl->IsEnabled(setting, enabled);
}


OSc_Error OSc_Setting_Is_Writable(OSc_Setting *setting, bool *writable)
{
	return setting->impl->IsWritable(setting, writable);
}


OSc_Error OSc_Setting_Get_Numeric_Constraint_Type(OSc_Setting *setting, OSc_Value_Constraint *constraintType)
{
	*constraintType = OSc_Value_Constraint_None;
	enum OScDev_ValueConstraint dConstraintType;
	OScDev_Error err;
	if (OSc_Check_Error(err, setting->impl->GetNumericConstraintType(setting, &dConstraintType)))
		return err;
	*constraintType = (OSc_Value_Constraint)dConstraintType;
	return OSc_Error_OK;
}


OSc_Error OSc_Setting_Get_String_Value(OSc_Setting *setting, char *value)
{
	return setting->impl->GetString(setting, value);
}


OSc_Error OSc_Setting_Set_String_Value(OSc_Setting *setting, const char *value)
{
	return setting->impl->SetString(setting, value);
}


OSc_Error OSc_Setting_Get_Bool_Value(OSc_Setting *setting, bool *value)
{
	return setting->impl->GetBool(setting, value);
}


OSc_Error OSc_Setting_Set_Bool_Value(OSc_Setting *setting, bool value)
{
	return setting->impl->SetBool(setting, value);
}


OSc_Error OSc_Setting_Get_Int32_Value(OSc_Setting *setting, int32_t *value)
{
	return setting->impl->GetInt32(setting, value);
}


OSc_Error OSc_Setting_Set_Int32_Value(OSc_Setting *setting, int32_t value)
{
	return setting->impl->SetInt32(setting, value);
}


OSc_Error OSc_Setting_Get_Int32_Range(OSc_Setting *setting, int32_t *min, int32_t *max)
{
	return setting->impl->GetInt32Range(setting, min, max);
}


OSc_Error OSc_Setting_Get_Int32_Discrete_Values(OSc_Setting *setting, int32_t **values, size_t *count)
{
	return setting->impl->GetInt32DiscreteValues(setting, values, count);
}


OSc_Error OSc_Setting_Get_Float64_Value(OSc_Setting *setting, double *value)
{
	return setting->impl->GetFloat64(setting, value);
}


OSc_Error OSc_Setting_Set_Float64_Value(OSc_Setting *setting, double value)
{
	return setting->impl->SetFloat64(setting, value);
}


OSc_Error OSc_Setting_Get_Float64_Range(OSc_Setting *setting, double *min, double *max)
{
	return setting->impl->GetFloat64Range(setting, min, max);
}


OSc_Error OSc_Setting_Get_Float64_Discrete_Values(OSc_Setting *setting, double **values, size_t *count)
{
	return setting->impl->GetFloat64DiscreteValues(setting, values, count);
}


OSc_Error OSc_Setting_Get_Enum_Value(OSc_Setting *setting, uint32_t *value)
{
	return setting->impl->GetEnum(setting, value);
}


OSc_Error OSc_Setting_Set_Enum_Value(OSc_Setting *setting, uint32_t value)
{
	return setting->impl->SetEnum(setting, value);
}


OSc_Error OSc_Setting_Get_Enum_Num_Values(OSc_Setting *setting, uint32_t *count)
{
	return setting->impl->GetEnumNumValues(setting, count);
}


OSc_Error OSc_Setting_Get_Enum_Name_For_Value(OSc_Setting *setting, uint32_t value, char *name)
{
	return setting->impl->GetEnumNameForValue(setting, value, name);
}


OSc_Error OSc_Setting_Get_Enum_Value_For_Name(OSc_Setting *setting, uint32_t *value, const char *name)
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
	case OSc_Value_Type_Int32:
	case OSc_Value_Type_Float64:
		return OSc_Error_OK;
	}
	return OSc_Error_Wrong_Value_Type;
}


static OSc_Error DefaultGetString(OSc_Setting *setting, char *value)
{
	strcpy(value, "");
	if (setting->valueType != OSc_Value_Type_String)
		return OSc_Error_Wrong_Value_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultSetString(OSc_Setting *setting, const char *value)
{
	if (setting->valueType != OSc_Value_Type_String)
		return OSc_Error_Wrong_Value_Type;

	bool writable;
	OSc_Return_If_Error(OSc_Setting_Is_Writable(setting, &writable));
	if (!writable)
		return OSc_Error_Setting_Not_Writable;

	return OSc_Error_OK;
}


static OSc_Error DefaultGetBool(OSc_Setting *setting, bool *value)
{
	*value = false;
	if (setting->valueType != OSc_Value_Type_Bool)
		return OSc_Error_Wrong_Value_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultSetBool(OSc_Setting *setting, bool value)
{
	if (setting->valueType != OSc_Value_Type_Bool)
		return OSc_Error_Wrong_Value_Type;

	bool writable;
	OSc_Return_If_Error(OSc_Setting_Is_Writable(setting, &writable));
	if (!writable)
		return OSc_Error_Setting_Not_Writable;

	return OSc_Error_OK;
}


static OSc_Error DefaultGetInt32(OSc_Setting *setting, int32_t *value)
{
	*value = 0;
	if (setting->valueType != OSc_Value_Type_Int32)
		return OSc_Error_Wrong_Value_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultSetInt32(OSc_Setting *setting, int32_t value)
{
	if (setting->valueType != OSc_Value_Type_Int32)
		return OSc_Error_Wrong_Value_Type;

	bool writable;
	OSc_Return_If_Error(OSc_Setting_Is_Writable(setting, &writable));
	if (!writable)
		return OSc_Error_Setting_Not_Writable;

	// TODO Check constraint

	return OSc_Error_OK;
}


static OSc_Error DefaultGetInt32Range(OSc_Setting *setting, int32_t *min, int32_t *max)
{
	*min = *max = 0;
	if (setting->valueType != OSc_Value_Type_Int32)
		return OSc_Error_Wrong_Value_Type;
	OSc_Value_Constraint constraint;
	OSc_Return_If_Error(OSc_Setting_Get_Numeric_Constraint_Type(setting, &constraint));
	if (constraint != OSc_Value_Constraint_Range)
		return OSc_Error_Wrong_Constraint_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultGetInt32DiscreteValues(OSc_Setting *setting, int32_t **values, size_t *count)
{
	static int32_t v = { 0 };
	*values = &v;
	*count = 1;
	if (setting->valueType != OSc_Value_Type_Int32)
		return OSc_Error_Wrong_Value_Type;
	OSc_Value_Constraint constraint;
	OSc_Return_If_Error(OSc_Setting_Get_Numeric_Constraint_Type(setting, &constraint));
	if (constraint != OSc_Value_Constraint_Discrete_Values)
		return OSc_Error_Wrong_Constraint_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultGetFloat64(OSc_Setting *setting, double *value)
{
	*value = 0.0;
	if (setting->valueType != OSc_Value_Type_Float64)
		return OSc_Error_Wrong_Value_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultSetFloat64(OSc_Setting *setting, double value)
{
	if (setting->valueType != OSc_Value_Type_Float64)
		return OSc_Error_Wrong_Value_Type;

	bool writable;
	OSc_Return_If_Error(OSc_Setting_Is_Writable(setting, &writable));
	if (!writable)
		return OSc_Error_Setting_Not_Writable;

	// TODO Check constraint

	return OSc_Error_OK;
}


static OSc_Error DefaultGetFloat64Range(OSc_Setting *setting, double *min, double *max)
{
	*min = *max = 0.0;
	if (setting->valueType != OSc_Value_Type_Float64)
		return OSc_Error_Wrong_Value_Type;
	OSc_Value_Constraint constraint;
	OSc_Return_If_Error(OSc_Setting_Get_Numeric_Constraint_Type(setting, &constraint));
	if (constraint != OSc_Value_Constraint_Range)
		return OSc_Error_Wrong_Constraint_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultGetFloat64DiscreteValues(OSc_Setting *setting, double **values, size_t *count)
{
	static double v = { 0.0 };
	*values = &v;
	*count = 1;
	if (setting->valueType != OSc_Value_Type_Float64)
		return OSc_Error_Wrong_Value_Type;
	OSc_Value_Constraint constraint;
	OSc_Return_If_Error(OSc_Setting_Get_Numeric_Constraint_Type(setting, &constraint));
	if (constraint != OSc_Value_Constraint_Discrete_Values)
		return OSc_Error_Wrong_Constraint_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultGetEnum(OSc_Setting *setting, uint32_t *value)
{
	*value = 0;
	if (setting->valueType != OSc_Value_Type_Enum)
		return OSc_Error_Wrong_Value_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultSetEnum(OSc_Setting *setting, uint32_t value)
{
	if (setting->valueType != OSc_Value_Type_Int32)
		return OSc_Error_Wrong_Value_Type;

	bool writable;
	OSc_Return_If_Error(OSc_Setting_Is_Writable(setting, &writable));
	if (!writable)
		return OSc_Error_Setting_Not_Writable;

	// TODO Range Check

	return OSc_Error_OK;
}


static OSc_Error DefaultGetEnumNumValues(OSc_Setting *setting, uint32_t *count)
{
	*count = 1;
	if (setting->valueType != OSc_Value_Type_Enum)
		return OSc_Error_Wrong_Value_Type;
	return OSc_Error_OK;
}


static OSc_Error DefaultGetEnumNameForValue(OSc_Setting *setting, uint32_t value, char *name)
{
	strcpy(name, "");

	if (setting->valueType != OSc_Value_Type_Enum)
		return OSc_Error_Wrong_Value_Type;

	// TODO Check range

	snprintf(name, OSc_MAX_STR_LEN, "Value-%u", value);
	return OSc_Error_OK;
}


static OSc_Error DefaultGetEnumValueForName(OSc_Setting *setting, uint32_t *value, const char *name)
{
	value = 0;

	if (setting->valueType != OSc_Value_Type_Enum)
		return OSc_Error_Wrong_Value_Type;

	if (strstr(name, "Value-") != 0)
		return OSc_Error_Unknown_Enum_Value_Name;
	long parsedNum = atol(name + strlen("Value-"));

	// TODO Check range

	*value = parsedNum;
	return OSc_Error_OK;
}


OSc_Error OSc_Setting_Create(OSc_Setting **setting, const char *name, OSc_Value_Type valueType,
	struct OScDev_SettingImpl *impl, void *data)
{
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

	*setting = calloc(1, sizeof(OSc_Setting));
	(*setting)->impl = impl;
	(*setting)->implData = data;
	(*setting)->valueType = valueType;
	strncpy((*setting)->name, name, OSc_MAX_STR_LEN);
	return OSc_Error_OK;
}


OSc_Error OSc_Setting_NumericConstraintRange(OSc_Setting *setting, OSc_Value_Constraint *constraintType)
{
	*constraintType = OSc_Value_Constraint_Range;
	return OSc_Error_OK;
}


OSc_Error OSc_Setting_NumericConstraintDiscreteValues(OSc_Setting *setting, OSc_Value_Constraint *constraintType)
{
	*constraintType = OSc_Value_Constraint_Discrete_Values;
	return OSc_Error_OK;
}


void *OSc_Setting_GetImplData(OSc_Setting *setting)
{
	return setting->implData;
}