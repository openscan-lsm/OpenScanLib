#include "OScNIDAQDevicePrivate.h"

#include <NIDAQmx.h>

#include <string.h>

const char* const PROPERTY_VALUE_Channel1 = "Channel1";
const char* const PROPERTY_VALUE_Channel2 = "Channel2";
const char* const PROPERTY_VALUE_Channel3 = "Channel3";
const char* const PROPERTY_VALUE_Channel1and2 = "Channel1and2";
const char* const PROPERTY_VALUE_Channel1and3 = "Channel1and3";
const char* const PROPERTY_VALUE_Channel1and2and3 = "Channels1-3";


// For most settings, we set the setting's implData to the device.
// This function can then be used to retrieve the device implData.
static inline struct OScNIDAQPrivateData *GetSettingDeviceData(OScDev_Setting *setting)
{
	return (struct OScNIDAQPrivateData *)OScDev_Device_GetImplData((OScDev_Device *)OScDev_Setting_GetImplData(setting));
}


OScDev_Error GetNumericConstraintTypeImpl_DiscreteValues(OScDev_Setting *setting, OScDev_ValueConstraint *constraintType)
{
	*constraintType = OScDev_ValueConstraint_DiscreteValues;
	return OScDev_OK;
}


OScDev_Error GetNumericConstraintTypeImpl_Range(OScDev_Setting *setting, OScDev_ValueConstraint *constraintType)
{
	*constraintType = OScDev_ValueConstraint_Range;
	return OScDev_OK;
}


static OScDev_Error GetLineDelay(OScDev_Setting *setting, int32_t *value)
{
	*value = GetSettingDeviceData(setting)->lineDelay;

	return OScDev_OK;
}


static OScDev_Error SetLineDelay(OScDev_Setting *setting, int32_t value)
{
	GetSettingDeviceData(setting)->lineDelay = value;
	
	GetSettingDeviceData(setting)->clockConfig.mustReconfigureTiming = true;
	GetSettingDeviceData(setting)->scannerConfig.mustReconfigureTiming = true;
	GetSettingDeviceData(setting)->clockConfig.mustRewriteOutput = true;
	GetSettingDeviceData(setting)->scannerConfig.mustRewriteOutput = true;

	return OScDev_OK;
}


static OScDev_Error GetLineDelayRange(OScDev_Setting *setting, int32_t *min, int32_t *max)
{
	*min = 1;
	*max = 200;
	return OScDev_OK;
}


OScDev_SettingImpl SettingImpl_LineDelay = {
	.GetInt32 = GetLineDelay,
	.SetInt32 = SetLineDelay,
	.GetNumericConstraintType = GetNumericConstraintTypeImpl_Range,
	.GetInt32Range = GetLineDelayRange,
};


static OScDev_Error GetBinFactor(OScDev_Setting *setting, int32_t *value)
{
	*value = GetSettingDeviceData(setting)->binFactor;
	return OScDev_OK;
}


// OnBinFactor
static OScDev_Error SetBinFactor(OScDev_Setting *setting, int32_t value)
{
	GetSettingDeviceData(setting)->binFactor = value;

	GetSettingDeviceData(setting)->clockConfig.mustReconfigureTiming = true;
	GetSettingDeviceData(setting)->scannerConfig.mustReconfigureTiming = true;
	GetSettingDeviceData(setting)->detectorConfig.mustReconfigureTiming = true;
	GetSettingDeviceData(setting)->detectorConfig.mustReconfigureCallback = true;

	return OScDev_OK;
}


static OScDev_Error GetBinFactorRange(OScDev_Setting *setting, int32_t *min, int32_t *max)
{
	*min = 1;
	*max = 25;
	return OScDev_OK;
}

static OScDev_SettingImpl SettingImpl_BinFactor = {
	.GetInt32 = GetBinFactor,
	.SetInt32 = SetBinFactor,
	.GetNumericConstraintType = GetNumericConstraintTypeImpl_Range,
	.GetInt32Range = GetBinFactorRange,
};

static OScDev_Error GetAcqBufferSize(OScDev_Setting *setting, int32_t *value)
{
	*value = GetSettingDeviceData(setting)->numLinesToBuffer;
	return OScDev_OK;
}

// OnAcqBufferSize
static OScDev_Error SetAcqBufferSize(OScDev_Setting *setting, int32_t value)
{
	GetSettingDeviceData(setting)->numLinesToBuffer = value;

	GetSettingDeviceData(setting)->detectorConfig.mustReconfigureCallback = true;

	return OScDev_OK;
}

static OScDev_Error GetAcqBufferSizeValues(OScDev_Setting *setting, OScDev_NumArray **values)
{
	static const uint32_t v[] = {
		2,
		4,
		8,
		16,
		32,
		64,
		128,
		256,
		0 // End mark
	};
	*values = OScDev_NumArray_Create();
	for (size_t i = 0; v[i] != 0; ++i) {
		OScDev_NumArray_Append(*values, v[i]);
	}
	return OScDev_OK;
}

static OScDev_SettingImpl SettingImpl_AcqBufferSize = {
	.GetInt32 = GetAcqBufferSize,
	.SetInt32 = SetAcqBufferSize,
	.GetNumericConstraintType = GetNumericConstraintTypeImpl_DiscreteValues,
	.GetInt32DiscreteValues = GetAcqBufferSizeValues,
};

static OScDev_Error GetInputVoltageRange(OScDev_Setting *setting, double *value)
{
	*value = GetSettingDeviceData(setting)->inputVoltageRange;
	return OScDev_OK;
}


static OScDev_Error SetInputVoltageRange(OScDev_Setting *setting, double value)
{
	GetSettingDeviceData(setting)->inputVoltageRange = value;
	return OScDev_OK;
}


static OScDev_Error GetInputVoltageRangeValues(OScDev_Setting *setting, OScDev_NumArray **values)
{
	static const double v[] = {
		1.0000,
		2.0000,
		5.0000,
		10.0000,
		0.0 // End mark
	};
	*values = OScDev_NumArray_Create();
	for (size_t i = 0; v[i] != 0.0; ++i) {
		OScDev_NumArray_Append(*values, v[i]);
	}
	return OScDev_OK;
}


static OScDev_SettingImpl SettingImpl_InputVoltageRange = {
	.GetFloat64 = GetInputVoltageRange,
	.SetFloat64 = SetInputVoltageRange,
	.GetNumericConstraintType = GetNumericConstraintTypeImpl_DiscreteValues,
	.GetFloat64DiscreteValues = GetInputVoltageRangeValues,
};


static OScDev_Error GetChannels(OScDev_Setting *setting, uint32_t *value)
{
	*value = GetSettingDeviceData(setting)->channels;
	return OScDev_OK;
}


static OScDev_Error SetChannels(OScDev_Setting *setting, uint32_t value)
{
	GetSettingDeviceData(setting)->channels = value;

	// Force recreation of detector task next time
	OScDev_Device *device = (OScDev_Device *)OScDev_Setting_GetImplData(setting);
	OScDev_Error err = ShutdownDetector(device,
		&GetSettingDeviceData(setting)->detectorConfig);

	return OScDev_OK;
}


static OScDev_Error GetChannelsNumValues(OScDev_Setting *setting, uint32_t *count)
{
	*count = CHANNELS_NUM_VALUES;
	return OScDev_OK;
}


static OScDev_Error GetChannelsNameForValue(OScDev_Setting *setting, uint32_t value, char *name)
{
	switch (value)
	{
	case CHANNEL1:
		strcpy(name, "Channel1");
		break;
	case CHANNEL2:
		strcpy(name, "Channel2");
		break;
	case CHANNEL3:
		strcpy(name, "Channel3");
		break;
		break;
	case CHANNELS_1_AND_2:
		strcpy(name, "Channel_1_and_2");
		break;
	case CHANNELS_1_AND_3:
		strcpy(name, "Channel_1_and_3");
		break;
	case CHANNELS1_2_3:
		strcpy(name, "Channel_1_2_3");
		break;
	default:
		strcpy(name, "");
		return OScDev_Error_Unknown;
	}

	OScDev_Device *device = (OScDev_Device *)OScDev_Setting_GetImplData(setting);
	OScDev_Error err;
	if (OScDev_CHECK(err, GetSelectedDispChannels(device))) {
		OScDev_Log_Error(device, "Fail to get selected disp channels");
	}
	return OScDev_OK;
}

static OScDev_Error GetChannelsValueForName(OScDev_Setting *setting, uint32_t *value, const char *name)
{
	if (!strcmp(name, "Channel1"))
		*value = CHANNEL1;
	else if (!strcmp(name, "Channel2"))
		*value = CHANNEL2;
	else if (!strcmp(name, "Channel3"))
		*value = CHANNEL3;
	else if (!strcmp(name, "Channel_1_and_2"))
		*value = CHANNELS_1_AND_2;
	else if (!strcmp(name, "Channel_1_and_3"))
		*value = CHANNELS_1_AND_3;
	else if (!strcmp(name, "Channel_1_2_3"))
		*value = CHANNELS1_2_3;
	else
		return OScDev_Error_Unknown;
	return OScDev_OK;
}


static OScDev_SettingImpl SettingImpl_Channels = {
	.GetEnum = GetChannels,
	.SetEnum = SetChannels,
	.GetEnumNumValues = GetChannelsNumValues,
	.GetEnumNameForValue = GetChannelsNameForValue,
	.GetEnumValueForName = GetChannelsValueForName,
};

static OScDev_Error GetScannerOnly(OScDev_Setting *setting, bool *value)
{
	*value = GetSettingDeviceData(setting)->scannerOnly;
	return OScDev_OK;
}

static OScDev_Error SetScannerOnly(OScDev_Setting *setting, bool value)
{
	GetSettingDeviceData(setting)->scannerOnly = value;
	return OScDev_OK;
}

static OScDev_SettingImpl SettingImpl_ScannerOnly = {
	.GetBool = GetScannerOnly,
	.SetBool = SetScannerOnly,
};


struct OffsetSettingData
{
	OScDev_Device *device;
	int axis; // 0 = x, 1 = y
};


static OScDev_Error GetOffset(OScDev_Setting *setting, double *value)
{
	struct OffsetSettingData *data = (struct OffsetSettingData *)OScDev_Setting_GetImplData(setting);
	*value = GetData(data->device)->offsetXY[data->axis];
	return OScDev_OK;
}


static OScDev_Error SetOffset(OScDev_Setting *setting, double value)
{
	struct OffsetSettingData *data = (struct OffsetSettingData *)OScDev_Setting_GetImplData(setting);
	GetData(data->device)->offsetXY[data->axis] = value;

	GetData(data->device)->clockConfig.mustRewriteOutput = true;
	GetData(data->device)->scannerConfig.mustRewriteOutput = true;

	return OScDev_OK;
}


static OScDev_Error GetOffsetRange(OScDev_Setting *setting, double *min, double *max)
{
	/*The galvoOffsetX and galvoOffsetY variables are expressed  in optical degrees
	This is a rough correspondence - it likely needs to be calibrated to the actual
	sensitivity of the galvos*/
	*min = -5.0;
	*max = +5.0;
	return OScDev_OK;
}


static void ReleaseOffset(OScDev_Setting *setting)
{
	struct OffsetSettingData *data = OScDev_Setting_GetImplData(setting);
	free(data);
}


static OScDev_SettingImpl SettingImpl_Offset = {
	.GetFloat64 = GetOffset,
	.SetFloat64 = SetOffset,
	.GetNumericConstraintType = GetNumericConstraintTypeImpl_Range,
	.GetFloat64Range = GetOffsetRange,
	.Release = ReleaseOffset,
};


OScDev_Error NIDAQMakeSettings(OScDev_Device *device, OScDev_PtrArray **settings)
{
	OScDev_Error err = OScDev_OK;
	*settings = OScDev_PtrArray_Create();

	OScDev_Setting *lineDelay;
	if (OScDev_CHECK(err, OScDev_Setting_Create(&lineDelay, "Line Delay (pixels)", OScDev_ValueType_Int32,
		&SettingImpl_LineDelay, device)))
		goto error;
	OScDev_PtrArray_Append(*settings, lineDelay);

	for (int i = 0; i < 2; ++i)
	{
		OScDev_Setting *offset;
		struct OffsetSettingData *data = malloc(sizeof(struct OffsetSettingData));
		data->device = device;
		data->axis = i;
		const char *name = i == 0 ? "GalvoOffsetX (degree)" : "GalvoOffsetY (degree)";
		if (OScDev_CHECK(err, OScDev_Setting_Create(&offset, name, OScDev_ValueType_Float64,
			&SettingImpl_Offset, data)))
			goto error;
		OScDev_PtrArray_Append(*settings, offset);
	}

	OScDev_Setting *binFactor;
	if (OScDev_CHECK(err, OScDev_Setting_Create(&binFactor, "Bin Factor", OScDev_ValueType_Int32,
		&SettingImpl_BinFactor, device)))
		goto error;
	OScDev_PtrArray_Append(*settings, binFactor);

	OScDev_Setting *numLinesToBuffer;
	if (OScDev_CHECK(err, OScDev_Setting_Create(&numLinesToBuffer, "Acq Buffer Size (lines)", OScDev_ValueType_Int32,
		&SettingImpl_AcqBufferSize, device)))
		goto error;
	OScDev_PtrArray_Append(*settings, numLinesToBuffer);

	OScDev_Setting *channels;
	if (OScDev_CHECK(err, OScDev_Setting_Create(&channels, "Channels", OScDev_ValueType_Enum,
		&SettingImpl_Channels, device)))
		goto error;
	OScDev_PtrArray_Append(*settings, channels);

	OScDev_Setting *inputVoltageRange;
	if (OScDev_CHECK(err, OScDev_Setting_Create(&inputVoltageRange, "Input Voltage Range", OScDev_ValueType_Float64,
		&SettingImpl_InputVoltageRange, device)))
		goto error;
	OScDev_PtrArray_Append(*settings, inputVoltageRange);

	OScDev_Setting *scannerOnly;
	if (OScDev_CHECK(err, OScDev_Setting_Create(&scannerOnly, "ScannerOnly", OScDev_ValueType_Bool,
		&SettingImpl_ScannerOnly, device)))
		goto error;
	OScDev_PtrArray_Append(*settings, scannerOnly); // TODO Remove when supported by OpenScanLib

	return OScDev_OK;

error:
	for (size_t i = 0; i < OScDev_PtrArray_Size(*settings); ++i) {
		OScDev_Setting_Destroy(OScDev_PtrArray_At(*settings, i));
	}
	OScDev_PtrArray_Destroy(*settings);
	*settings = NULL;
	return err;
}

static OScDev_Error GetSelectedDispChannels(OScDev_Device *device)
{
	// clear selectedDispChan
	GetData(device)->selectedDispChan_ = calloc(OSc_Total_Channel_Num * (OScDev_MAX_STR_LEN + 1), sizeof(char));

	switch (GetData(device)->channels)
	{
	case CHANNEL1:
		GetData(device)->selectedDispChan_[0] = PROPERTY_VALUE_Channel1;
		GetData(device)->channelCount = 1;
		break;
	case CHANNEL2:
		GetData(device)->selectedDispChan_[0] = PROPERTY_VALUE_Channel2;
		GetData(device)->channelCount = 1;
		break;
	case CHANNEL3:
		GetData(device)->selectedDispChan_[0] = PROPERTY_VALUE_Channel3;
		GetData(device)->channelCount = 1;
		break;
	case CHANNELS_1_AND_2:
		GetData(device)->selectedDispChan_[0] = PROPERTY_VALUE_Channel1;
		GetData(device)->selectedDispChan_[1] = PROPERTY_VALUE_Channel2;
		GetData(device)->channelCount = 2;
		break;
	case CHANNELS_1_AND_3:
		GetData(device)->selectedDispChan_[0] =  PROPERTY_VALUE_Channel1;
		GetData(device)->selectedDispChan_[1] = PROPERTY_VALUE_Channel3;
		GetData(device)->channelCount = 2;
		break;
	case CHANNELS1_2_3:
		GetData(device)->selectedDispChan_[0] = PROPERTY_VALUE_Channel1;
		GetData(device)->selectedDispChan_[1] = PROPERTY_VALUE_Channel2;
		GetData(device)->selectedDispChan_[2] = PROPERTY_VALUE_Channel3;
		GetData(device)->channelCount = 3;
		break;
	}

	return OScDev_OK;
}
