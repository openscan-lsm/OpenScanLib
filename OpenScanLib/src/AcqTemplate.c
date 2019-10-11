#include "OpenScanLibPrivate.h"

#include <math.h>


struct OScInternal_AcqTemplate
{
	OSc_LSM *lsm;
	uint32_t numberOfFrames;

	OSc_Setting *pixelRateSetting;
	OSc_Setting *resolutionSetting;
	OSc_Setting *zoomFactorSetting;
	OSc_Setting *magnificationSetting;

	double pixelRateHz;
	uint32_t resolution;
	double zoomFactor;

	uint32_t xOffset;
	uint32_t yOffset;
	uint32_t width;
	uint32_t height;
};


// Built-in absolute limits
OScDev_STATIC_NUM_RANGE_CONTINUOUS(PixelRateRange, 1e-3, 1e10);
OScDev_STATIC_NUM_RANGE_CONTINUOUS(ResolutionRange, 1, INT32_MAX);
OScDev_STATIC_NUM_RANGE_CONTINUOUS(ZoomRange, 1e-6, 1e6);


static OScDev_NumRange *GetPixelRates(OSc_AcqTemplate *tmpl)
{
	OSc_Device *clockDevice = OSc_LSM_GetClockDevice(tmpl->lsm);
	OSc_Device *scannerDevice = OSc_LSM_GetScannerDevice(tmpl->lsm);
	OSc_Device *detectorDevice = OSc_LSM_GetDetectorDevice(tmpl->lsm);

	OScDev_NumRange *clockRange = OScInternal_Device_GetPixelRates(clockDevice);
	OScDev_NumRange *scannerRange = OScInternal_Device_GetPixelRates(scannerDevice);
	OScDev_NumRange *detectorRange = OScInternal_Device_GetPixelRates(detectorDevice);

	OScDev_NumRange *range = OScInternal_NumRange_Intersection4(
		clockRange, scannerRange, detectorRange, &PixelRateRange);

	OScInternal_NumRange_Destroy(detectorRange);
	OScInternal_NumRange_Destroy(scannerRange);
	OScInternal_NumRange_Destroy(clockRange);

	return range;
}


static double GetDefaultPixelRate(OSc_AcqTemplate *tmpl)
{
	OScDev_NumRange *range = GetPixelRates(tmpl);
	double ret = OScInternal_NumRange_ClosestValue(range, 1e6);
	OScInternal_NumRange_Destroy(range);
	return ret;
}


static OScDev_Error GetPixelRateConstraintType(OScDev_Setting *setting, OScDev_ValueConstraint *constraint)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	OScDev_NumRange *range = GetPixelRates(tmpl);
	if (OScInternal_NumRange_IsDiscrete(range)) {
		*constraint = OScDev_ValueConstraint_DiscreteValues;
	}
	else {
		*constraint = OScDev_ValueConstraint_Range;
	}
	OScInternal_NumRange_Destroy(range);
	return OSc_Error_OK;
}


static OScDev_Error GetPixelRateDiscreteValues(OScDev_Setting *setting, OScDev_NumArray **values)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	OScDev_NumRange *range = GetPixelRates(tmpl);
	if (!OScInternal_NumRange_IsDiscrete(range)) {
		return OSc_Error_Wrong_Constraint_Type;
	}
	*values = OScInternal_NumRange_DiscreteValues(range);
	OScInternal_NumRange_Destroy(range);
	return OSc_Error_OK;
}


static OScDev_Error GetPixelRateRange(OScDev_Setting *setting, double *min, double *max)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	OScDev_NumRange *range = GetPixelRates(tmpl);
	if (OScInternal_NumRange_IsDiscrete(range)) {
		return OSc_Error_Wrong_Constraint_Type;
	}
	*min = OScInternal_NumRange_Min(range);
	*max = OScInternal_NumRange_Max(range);
	OScInternal_NumRange_Destroy(range);
	return OSc_Error_OK;
}


static OScDev_Error GetPixelRate(OScDev_Setting *setting, double *value)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	*value = tmpl->pixelRateHz;
	return OSc_Error_OK;
}


static OScDev_Error SetPixelRate(OScDev_Setting *setting, double value)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	tmpl->pixelRateHz = value;
	return OSc_Error_OK;
}


static OScDev_SettingImpl PixelRateSettingImpl = {
	.GetNumericConstraintType = GetPixelRateConstraintType,
	.GetFloat64 = GetPixelRate,
	.SetFloat64 = SetPixelRate,
	.GetFloat64Range = GetPixelRateRange,
	.GetFloat64DiscreteValues = GetPixelRateDiscreteValues,
};


static OScDev_NumRange *GetResolutions(OSc_AcqTemplate *tmpl)
{
	OSc_Device *scannerDevice = OSc_LSM_GetScannerDevice(tmpl->lsm);
	OScDev_NumRange *scannerRange = OScInternal_Device_GetResolutions(scannerDevice);

	OScDev_NumRange *range = OScInternal_NumRange_Intersection(
		scannerRange, &ResolutionRange);

	// TODO We should also take intersection with clock/scanner/detector raster
	// widths and heights, to ensure that full frame acquisition will work.

	OScInternal_NumRange_Destroy(scannerRange);

	return range;
}


static int32_t GetDefaultResolution(OSc_AcqTemplate *tmpl)
{
	OScDev_NumRange *range = GetResolutions(tmpl);
	int32_t ret = (int32_t)OScInternal_NumRange_ClosestValue(range, 512);
	OScInternal_NumRange_Destroy(range);
	return ret;
}


static OScDev_Error GetResolutionConstraintType(OScDev_Setting *setting, OScDev_ValueConstraint *constraint)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	OScDev_NumRange *range = GetResolutions(tmpl);
	if (OScInternal_NumRange_IsDiscrete(range)) {
		*constraint = OScDev_ValueConstraint_DiscreteValues;
	}
	else {
		*constraint = OScDev_ValueConstraint_Range;
	}
	OScInternal_NumRange_Destroy(range);
	return OSc_Error_OK;
}


static OScDev_Error GetResolutionDiscreteValues(OScDev_Setting *setting, OScDev_NumArray **values)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	OScDev_NumRange *range = GetResolutions(tmpl);
	if (!OScInternal_NumRange_IsDiscrete(range)) {
		return OSc_Error_Wrong_Constraint_Type;
	}
	*values = OScInternal_NumRange_DiscreteValues(range);
	OScInternal_NumRange_Destroy(range);
	return OSc_Error_OK;
}


static OScDev_Error GetResolutionRange(OScDev_Setting *setting, int32_t *min, int32_t *max)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	OScDev_NumRange *range = GetResolutions(tmpl);
	if (OScInternal_NumRange_IsDiscrete(range)) {
		return OSc_Error_Wrong_Constraint_Type;
	}
	// Use ceil()/floor() only for safety; values are supposed to be integers
	*min = (int32_t)ceil(OScInternal_NumRange_Min(range));
	*max = (int32_t)floor(OScInternal_NumRange_Max(range));
	OScInternal_NumRange_Destroy(range);
	return OSc_Error_OK;
}


static OScDev_Error GetResolution(OScDev_Setting *setting, int32_t *value)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	*value = tmpl->resolution;
	return OSc_Error_OK;
}


static OScDev_Error SetResolution(OScDev_Setting *setting, int32_t value)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	if (tmpl->resolution != value) {
		tmpl->resolution = value;
		tmpl->xOffset = tmpl->yOffset = 0;
		tmpl->width = tmpl->height = value;
		OScInternal_Setting_Invalidate(tmpl->magnificationSetting);
	}
	return OSc_Error_OK;
}


static OScDev_SettingImpl ResolutionSettingImpl = {
	.GetNumericConstraintType = GetResolutionConstraintType,
	.GetInt32 = GetResolution,
	.SetInt32 = SetResolution,
	.GetInt32Range = GetResolutionRange,
	.GetInt32DiscreteValues = GetResolutionDiscreteValues,
};


static OScDev_NumRange *GetZooms(OSc_AcqTemplate *tmpl)
{
	OSc_Device *scannerDevice = OSc_LSM_GetScannerDevice(tmpl->lsm);
	OScDev_NumRange *scannerRange = OScInternal_Device_GetZooms(scannerDevice);

	OScDev_NumRange *range = OScInternal_NumRange_Intersection(
		scannerRange, &ZoomRange);

	OScInternal_NumRange_Destroy(scannerRange);

	return range;
}


static double GetDefaultZoom(OSc_AcqTemplate *tmpl)
{
	OScDev_NumRange *range = GetZooms(tmpl);
	double ret = OScInternal_NumRange_ClosestValue(range, 1.0);
	OScInternal_NumRange_Destroy(range);
	return ret;
}


static OScDev_Error GetZoomConstraintType(OScDev_Setting *setting, OScDev_ValueConstraint *constraint)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	OScDev_NumRange *range = GetZooms(tmpl);
	if (OScInternal_NumRange_IsDiscrete(range)) {
		*constraint = OScDev_ValueConstraint_DiscreteValues;
	}
	else {
		*constraint = OScDev_ValueConstraint_Range;
	}
	OScInternal_NumRange_Destroy(range);
	return OSc_Error_OK;
}


static OScDev_Error GetZoomDiscreteValues(OScDev_Setting *setting, OScDev_NumArray **values)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	OScDev_NumRange *range = GetZooms(tmpl);
	if (!OScInternal_NumRange_IsDiscrete(range)) {
		return OSc_Error_Wrong_Constraint_Type;
	}
	*values = OScInternal_NumRange_DiscreteValues(range);
	OScInternal_NumRange_Destroy(range);
	return OSc_Error_OK;
}


static OScDev_Error GetZoomRange(OScDev_Setting *setting, double *min, double *max)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	OScDev_NumRange *range = GetResolutions(tmpl);
	if (OScInternal_NumRange_IsDiscrete(range)) {
		return OSc_Error_Wrong_Constraint_Type;
	}
	*min = OScInternal_NumRange_Min(range);
	*max = OScInternal_NumRange_Max(range);
	OScInternal_NumRange_Destroy(range);
	return OSc_Error_OK;
}


static OScDev_Error GetZoom(OScDev_Setting *setting, double *value)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	*value = tmpl->zoomFactor;
	return OSc_Error_OK;
}


static OScDev_Error SetZoom(OScDev_Setting *setting, double value)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	if (value != tmpl->zoomFactor) {
		tmpl->zoomFactor = value;
		OScInternal_Setting_Invalidate(tmpl->magnificationSetting);
	}
	return OSc_Error_OK;
}


static OScDev_SettingImpl ZoomSettingImpl = {
	.GetNumericConstraintType = GetZoomConstraintType,
	.GetFloat64 = GetZoom,
	.SetFloat64 = SetZoom,
	.GetFloat64Range = GetZoomRange,
	.GetFloat64DiscreteValues = GetZoomDiscreteValues,
};


static OScDev_Error IsMagnificationWritable(OScDev_Setting *setting, bool *writable)
{
	*writable = false;
	return OSc_Error_OK;
}


static OScDev_Error GetMagnification(OScDev_Setting *setting, double *value)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	int32_t defaultResolution = GetDefaultResolution(tmpl);
	*value = tmpl->zoomFactor * tmpl->resolution / defaultResolution;
	return OSc_Error_OK;
}


static OScDev_SettingImpl MagnificationSettingImpl = {
	.IsWritable = IsMagnificationWritable,
	.GetFloat64 = GetMagnification,
};


OSc_Error OSc_AcqTemplate_Create(OSc_AcqTemplate **tmpl, OSc_LSM *lsm)
{
	if (!tmpl || !lsm)
		return OSc_Error_Illegal_Argument;

	*tmpl = calloc(1, sizeof(struct OScInternal_AcqTemplate));
	if (!*tmpl)
		return OSc_Error_Unknown;

	(*tmpl)->lsm = lsm;
	(*tmpl)->numberOfFrames = UINT32_MAX; // Infinite

	OSc_Error err;
	OSc_Setting *setting;

	if (OSc_CHECK_ERROR(err, OScInternal_Setting_Create(&setting, "PixelRateHz",
		OSc_ValueType_Float64, &PixelRateSettingImpl, *tmpl)))
		goto error;
	(*tmpl)->pixelRateSetting = setting;

	if (OSc_CHECK_ERROR(err, OScInternal_Setting_Create(&setting, "Resolution",
		OSc_ValueType_Int32, &ResolutionSettingImpl, *tmpl)))
		goto error;
	(*tmpl)->resolutionSetting = setting;

	if (OSc_CHECK_ERROR(err, OScInternal_Setting_Create(&setting, "ZoomFactor",
		OSc_ValueType_Float64, &ZoomSettingImpl, *tmpl)))
		goto error;
	(*tmpl)->zoomFactorSetting = setting;

	if (OSc_CHECK_ERROR(err, OScInternal_Setting_Create(&setting, "Magnification",
		OSc_ValueType_Float64, &MagnificationSettingImpl, *tmpl)))
		goto error;
	(*tmpl)->magnificationSetting = setting;

	(*tmpl)->pixelRateHz = GetDefaultPixelRate(*tmpl);
	(*tmpl)->resolution = GetDefaultResolution(*tmpl);
	(*tmpl)->zoomFactor = GetDefaultZoom(*tmpl);
	(*tmpl)->xOffset = (*tmpl)->yOffset = 0;
	(*tmpl)->width = (*tmpl)->height = (*tmpl)->resolution;

	return OSc_Error_OK;

error:
	OSc_AcqTemplate_Destroy(*tmpl);
	*tmpl = NULL;
	return err;
}


void OSc_AcqTemplate_Destroy(OSc_AcqTemplate *tmpl)
{
	if (tmpl) {
		OScInternal_Setting_Destroy(tmpl->zoomFactorSetting);
		OScInternal_Setting_Destroy(tmpl->resolutionSetting);
		OScInternal_Setting_Destroy(tmpl->pixelRateSetting);
		free(tmpl);
	}
}


OSc_LSM *OSc_AcqTemplate_GetLSM(OSc_AcqTemplate *tmpl)
{
	if (!tmpl)
		return NULL;
	return tmpl->lsm;
}


OSc_Error OSc_AcqTemplate_SetNumberOfFrames(OSc_AcqTemplate *tmpl, uint32_t numberOfFrames)
{
	if (!tmpl)
		return OSc_Error_Illegal_Argument;

	tmpl->numberOfFrames = numberOfFrames;
	return OSc_Error_OK;
}


uint32_t OSc_AcqTemplate_GetNumberOfFrames(OSc_AcqTemplate *tmpl)
{
	if (!tmpl)
		return 0;
	return tmpl->numberOfFrames;
}


OSc_Error OSc_AcqTemplate_GetPixelRateSetting(OSc_AcqTemplate *tmpl, OSc_Setting **setting)
{
	if (!tmpl)
		return OSc_Error_Illegal_Argument;
	*setting = tmpl->pixelRateSetting;
	return OSc_Error_OK;
}


OSc_Error OSc_AcqTemplate_GetResolutionSetting(OSc_AcqTemplate *tmpl, OSc_Setting **setting)
{
	if (!tmpl)
		return OSc_Error_Illegal_Argument;
	*setting = tmpl->resolutionSetting;
	return OSc_Error_OK;
}


OSc_Error OSc_AcqTemplate_GetZoomFactorSetting(OSc_AcqTemplate *tmpl, OSc_Setting **setting)
{
	if (!tmpl)
		return OSc_Error_Illegal_Argument;
	*setting = tmpl->zoomFactorSetting;
	return OSc_Error_OK;
}


OSc_Error OSc_AcqTemplate_GetMagnificationSetting(OSc_AcqTemplate *tmpl, OSc_Setting **setting)
{
	if (!tmpl)
		return OSc_Error_Illegal_Argument;
	*setting = tmpl->magnificationSetting;
	return OSc_Error_OK;
}


OSc_Error OSc_AcqTemplate_SetROI(OSc_AcqTemplate *tmpl, uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height)
{
	if (!tmpl)
		return OSc_Error_Illegal_Argument;
	if (xOffset >= tmpl->resolution || xOffset + width > tmpl->resolution ||
		yOffset >= tmpl->resolution || yOffset + height > tmpl->resolution)
		return OSc_Error_Unknown; // TODO Out of range
	if (width < 1 || height < 1)
		return OSc_Error_Unknown; // TODO Empty raster
	tmpl->xOffset = xOffset;
	tmpl->yOffset = yOffset;
	tmpl->width = width;
	tmpl->height = height;
	return OSc_Error_OK;
}


void OSc_AcqTemplate_ResetROI(OSc_AcqTemplate *tmpl)
{
	if (!tmpl)
		return;
	tmpl->xOffset = tmpl->yOffset = 0;
	tmpl->width = tmpl->height = tmpl->resolution;
}


OSc_Error OSc_AcqTemplate_GetROI(OSc_AcqTemplate *tmpl, uint32_t *xOffset, uint32_t *yOffset, uint32_t *width, uint32_t *height)
{
	if (!tmpl || !xOffset || !yOffset || !width || !height)
		return OSc_Error_Illegal_Argument;
	*xOffset = tmpl->xOffset;
	*yOffset = tmpl->yOffset;
	*width = tmpl->width;
	*height = tmpl->height;
	return OSc_Error_OK;
}


OSc_Error OSc_AcqTemplate_GetNumberOfChannels(OSc_AcqTemplate *tmpl, uint32_t *numberOfChannels)
{
	if (!tmpl || !numberOfChannels)
		return OSc_Error_Illegal_Argument;

	// This implementation is temporary; device-specific settings that affect
	// the number of channels should belong to the AcqTemplate and we need a
	// mechanism to allow the device implementation to compute the number of
	// channels with the AcqTemplate settings as sole input.
	OSc_Device *detectorDevice = OSc_LSM_GetDetectorDevice(tmpl->lsm);
	return OScInternal_Device_GetNumberOfChannels(detectorDevice, numberOfChannels);
}


OSc_Error OSc_AcqTemplate_GetBytesPerSample(OSc_AcqTemplate *tmpl, uint32_t *bytesPerSample)
{
	if (!tmpl || !bytesPerSample)
		return OSc_Error_Illegal_Argument;

	// This implementation is temporary; device-specific settings that affect
	// the sample format should belong to the AcqTemplate and we need a
	// mechanism to allow the device implementation to compute the sample
	// format with the AcqTemplate settings as sole input.
	OSc_Device *detectorDevice = OSc_LSM_GetDetectorDevice(tmpl->lsm);
	return OScInternal_Device_GetNumberOfChannels(detectorDevice, bytesPerSample);
}
