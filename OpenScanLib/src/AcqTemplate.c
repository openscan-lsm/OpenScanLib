#include "OpenScanLibPrivate.h"
#include "InternalErrors.h"

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


static OScDev_NumRange *GetPixelRates(OSc_AcqTemplate *tmpl)
{
	OSc_Device *clockDevice = OSc_LSM_GetClockDevice(tmpl->lsm);
	OSc_Device *scannerDevice = OSc_LSM_GetScannerDevice(tmpl->lsm);
	OSc_Device *detectorDevice = OSc_LSM_GetDetectorDevice(tmpl->lsm);

	OScDev_NumRange *clockRange = OScInternal_Device_GetPixelRates(clockDevice);
	OScDev_NumRange *scannerRange = OScInternal_Device_GetPixelRates(scannerDevice);
	OScDev_NumRange *detectorRange = OScInternal_Device_GetPixelRates(detectorDevice);
	OScDev_NumRange *maxRange = OScInternal_NumRange_CreateContinuous(1e-3, 1e10);

	OScDev_NumRange *range = OScInternal_NumRange_Intersection4(
		clockRange, scannerRange, detectorRange, maxRange);

	OScInternal_NumRange_Destroy(maxRange);
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
	return OScInternal_LegacyError_OK;
}


static OScDev_Error GetPixelRateDiscreteValues(OScDev_Setting *setting, OScDev_NumArray **values)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	OScDev_NumRange *range = GetPixelRates(tmpl);
	if (!OScInternal_NumRange_IsDiscrete(range)) {
		OScInternal_Error_ReturnAsCode(OScInternal_Error_WrongConstraintType());
	}
	*values = OScInternal_NumRange_DiscreteValues(range);
	OScInternal_NumRange_Destroy(range);
	return OScInternal_LegacyError_OK;
}


static OScDev_Error GetPixelRateRange(OScDev_Setting *setting, double *min, double *max)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	OScDev_NumRange *range = GetPixelRates(tmpl);
	if (OScInternal_NumRange_IsDiscrete(range)) {
		return OScInternal_Error_ReturnAsCode(OScInternal_Error_WrongConstraintType());
	}
	*min = OScInternal_NumRange_Min(range);
	*max = OScInternal_NumRange_Max(range);
	OScInternal_NumRange_Destroy(range);
	return OScInternal_LegacyError_OK;
}


static OScDev_Error GetPixelRate(OScDev_Setting *setting, double *value)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	*value = tmpl->pixelRateHz;
	return OScInternal_LegacyError_OK;
}


static OScDev_Error SetPixelRate(OScDev_Setting *setting, double value)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	tmpl->pixelRateHz = value;
	return OScInternal_LegacyError_OK;
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
	// A full-frame scan (ROI = [0, 0, res, res]) must always be possible, so
	// resolutions are limited by both scanner resolutions and clock/detector
	// raster width/height.

	OSc_Device *clockDevice = OSc_LSM_GetClockDevice(tmpl->lsm);
	OSc_Device *scannerDevice = OSc_LSM_GetScannerDevice(tmpl->lsm);
	OSc_Device *detectorDevice = OSc_LSM_GetDetectorDevice(tmpl->lsm);
	OScDev_NumRange *clockWidthRange = OScInternal_Device_GetRasterWidths(clockDevice);
	OScDev_NumRange *clockHeightRange = OScInternal_Device_GetRasterHeights(clockDevice);
	OScDev_NumRange *scannerRange = OScInternal_Device_GetResolutions(scannerDevice);
	OScDev_NumRange *detectorWidthRange = OScInternal_Device_GetRasterWidths(detectorDevice);
	OScDev_NumRange *detectorHeightRange = OScInternal_Device_GetRasterHeights(detectorDevice);
	OScDev_NumRange *maxRange = OScInternal_NumRange_CreateContinuous(1, INT32_MAX);

	OScDev_NumRange *range = OScInternal_NumRange_Intersection6(
		clockWidthRange, clockHeightRange, scannerRange,
		detectorWidthRange, detectorHeightRange, maxRange);

	OScInternal_NumRange_Destroy(maxRange);
	OScInternal_NumRange_Destroy(detectorHeightRange);
	OScInternal_NumRange_Destroy(detectorWidthRange);
	OScInternal_NumRange_Destroy(scannerRange);
	OScInternal_NumRange_Destroy(clockHeightRange);
	OScInternal_NumRange_Destroy(clockWidthRange);

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
	return OScInternal_LegacyError_OK;
}


static OScDev_Error GetResolutionDiscreteValues(OScDev_Setting *setting, OScDev_NumArray **values)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	OScDev_NumRange *range = GetResolutions(tmpl);
	if (!OScInternal_NumRange_IsDiscrete(range)) {
		return OScInternal_Error_ReturnAsCode(OScInternal_Error_WrongConstraintType());
	}
	*values = OScInternal_NumRange_DiscreteValues(range);
	OScInternal_NumRange_Destroy(range);
	return OScInternal_LegacyError_OK;
}


static OScDev_Error GetResolutionRange(OScDev_Setting *setting, int32_t *min, int32_t *max)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	OScDev_NumRange *range = GetResolutions(tmpl);
	if (OScInternal_NumRange_IsDiscrete(range)) {
		return OScDev_Error_Wrong_Constraint_Type;
	}
	// Use ceil()/floor() only for safety; values are supposed to be integers
	*min = (int32_t)ceil(OScInternal_NumRange_Min(range));
	*max = (int32_t)floor(OScInternal_NumRange_Max(range));
	OScInternal_NumRange_Destroy(range);
	return OScInternal_LegacyError_OK;
}


static OScDev_Error GetResolution(OScDev_Setting *setting, int32_t *value)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	*value = tmpl->resolution;
	return OScInternal_LegacyError_OK;
}


static OScDev_Error SetResolution(OScDev_Setting *setting, int32_t value)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	if (tmpl->resolution != value) {
		tmpl->resolution = value;
		OSc_AcqTemplate_ResetROI(tmpl);
		OScInternal_Setting_Invalidate(tmpl->magnificationSetting);
	}
	return OScInternal_LegacyError_OK;
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
	OScDev_NumRange *maxRange = OScInternal_NumRange_CreateContinuous(1e-6, 1e6);

	OScDev_NumRange *range = OScInternal_NumRange_Intersection(
		scannerRange, maxRange);

	OScInternal_NumRange_Destroy(maxRange);
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
	return OScInternal_LegacyError_OK;
}


static OScDev_Error GetZoomDiscreteValues(OScDev_Setting *setting, OScDev_NumArray **values)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	OScDev_NumRange *range = GetZooms(tmpl);
	if (!OScInternal_NumRange_IsDiscrete(range)) {
		return OScInternal_Error_ReturnAsCode(OScInternal_Error_WrongConstraintType());
	}
	*values = OScInternal_NumRange_DiscreteValues(range);
	OScInternal_NumRange_Destroy(range);
	return OScInternal_LegacyError_OK;
}


static OScDev_Error GetZoomRange(OScDev_Setting *setting, double *min, double *max)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	OScDev_NumRange *range = GetZooms(tmpl);
	if (OScInternal_NumRange_IsDiscrete(range)) {
		return OScInternal_Error_ReturnAsCode(OScInternal_Error_WrongConstraintType());
	}
	*min = OScInternal_NumRange_Min(range);
	*max = OScInternal_NumRange_Max(range);
	OScInternal_NumRange_Destroy(range);
	return OScInternal_LegacyError_OK;
}


static OScDev_Error GetZoom(OScDev_Setting *setting, double *value){
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	*value = tmpl->zoomFactor;
	return OScInternal_LegacyError_OK;
}


static OScDev_Error SetZoom(OScDev_Setting *setting, double value)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	if (value != tmpl->zoomFactor) {
		tmpl->zoomFactor = value;
		OScInternal_Setting_Invalidate(tmpl->magnificationSetting);
	}
	return OScInternal_LegacyError_OK;
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
	return OScInternal_LegacyError_OK;
}


static OScDev_Error GetMagnification(OScDev_Setting *setting, double *value)
{
	OSc_AcqTemplate *tmpl = OScInternal_Setting_GetImplData(setting);
	int32_t defaultResolution = GetDefaultResolution(tmpl);
	*value = tmpl->zoomFactor * tmpl->resolution / defaultResolution;
	return OScInternal_LegacyError_OK;
}


static OScDev_SettingImpl MagnificationSettingImpl = {
	.IsWritable = IsMagnificationWritable,
	.GetFloat64 = GetMagnification,
};


OSc_RichError *OSc_AcqTemplate_Create(OSc_AcqTemplate **tmpl, OSc_LSM *lsm)
{
	if (!tmpl || !lsm)
		return OScInternal_Error_IllegalArgument();

	*tmpl = calloc(1, sizeof(struct OScInternal_AcqTemplate));
	if (!*tmpl)
		return OScInternal_Error_OutOfMemory();

	(*tmpl)->lsm = lsm;
	(*tmpl)->numberOfFrames = UINT32_MAX; // Infinite

	OSc_RichError *err;
	OSc_Setting *setting;

	err = OScInternal_Setting_Create(NULL, &setting, "PixelRateHz",
		OSc_ValueType_Float64, &PixelRateSettingImpl, *tmpl);
	if (err)
		goto error;
	(*tmpl)->pixelRateSetting = setting;

	err = OScInternal_Setting_Create(NULL, &setting, "Resolution",
		OSc_ValueType_Int32, &ResolutionSettingImpl, *tmpl);
	if (err)
		goto error;
	(*tmpl)->resolutionSetting = setting;

	err = OScInternal_Setting_Create(NULL, &setting, "ZoomFactor",
		OSc_ValueType_Float64, &ZoomSettingImpl, *tmpl);
	if (err)
		goto error;
	(*tmpl)->zoomFactorSetting = setting;

	err = OScInternal_Setting_Create(NULL, &setting, "Magnification",
		OSc_ValueType_Float64, &MagnificationSettingImpl, *tmpl);
	if (err)
		goto error;
	(*tmpl)->magnificationSetting = setting;

	(*tmpl)->pixelRateHz = GetDefaultPixelRate(*tmpl);
	(*tmpl)->resolution = GetDefaultResolution(*tmpl);
	(*tmpl)->zoomFactor = GetDefaultZoom(*tmpl);
	OSc_AcqTemplate_ResetROI(*tmpl);

	return OSc_OK;

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


OSc_RichError *OSc_AcqTemplate_SetNumberOfFrames(OSc_AcqTemplate *tmpl, uint32_t numberOfFrames)
{
	if (!tmpl)
		return OScInternal_Error_IllegalArgument();

	tmpl->numberOfFrames = numberOfFrames;
	return OSc_OK;
}


uint32_t OSc_AcqTemplate_GetNumberOfFrames(OSc_AcqTemplate *tmpl)
{
	if (!tmpl)
		return 0;
	return tmpl->numberOfFrames;
}


OSc_RichError *OSc_AcqTemplate_GetPixelRateSetting(OSc_AcqTemplate *tmpl, OSc_Setting **setting)
{
	if (!tmpl)
		return OScInternal_Error_IllegalArgument();
	*setting = tmpl->pixelRateSetting;
	return OSc_OK;
}


OSc_RichError *OSc_AcqTemplate_GetResolutionSetting(OSc_AcqTemplate *tmpl, OSc_Setting **setting)
{
	if (!tmpl)
		return OScInternal_Error_IllegalArgument();
	*setting = tmpl->resolutionSetting;
	return OSc_OK;
}


OSc_RichError *OSc_AcqTemplate_GetZoomFactorSetting(OSc_AcqTemplate *tmpl, OSc_Setting **setting)
{
	if (!tmpl)
		return OScInternal_Error_IllegalArgument();
	*setting = tmpl->zoomFactorSetting;
	return OSc_OK;
}


OSc_RichError *OSc_AcqTemplate_GetMagnificationSetting(OSc_AcqTemplate *tmpl, OSc_Setting **setting)
{
	if (!tmpl)
		return OScInternal_Error_IllegalArgument();
	*setting = tmpl->magnificationSetting;
	return OSc_OK;
}


OSc_RichError *OSc_AcqTemplate_SetROI(OSc_AcqTemplate *tmpl, uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height)
{
	if (!tmpl)
		OScInternal_Error_IllegalArgument();

	// If ROI scan is not supported by the scanner, only full frame is allowed
	if (!OScInternal_Device_IsROIScanSupported(OSc_LSM_GetScannerDevice(tmpl->lsm))) {
		if (xOffset > 0 || yOffset > 0 || width != tmpl->resolution || height != tmpl->resolution) {
			return OScInternal_Error_UnsupportedOperation();
		}
	}

	// Otherwise, ROI must fit in resolution and be allowed by clock and
	// detector raster size.
	if (xOffset >= tmpl->resolution || xOffset + width > tmpl->resolution ||
		yOffset >= tmpl->resolution || yOffset + height > tmpl->resolution)
		return OScInternal_Error_OutOfRange();
	if (width < 1 || height < 1)
		return OScInternal_Error_EmptyRaster();

	bool rasterSizeOk = true;

	OSc_Device *clockDevice = OSc_LSM_GetClockDevice(tmpl->lsm);
	OScInternal_NumRange *clockWidthRange = OScInternal_Device_GetRasterWidths(clockDevice);
	if (!OScInternal_NumRange_Contains(clockWidthRange, width)) {
		rasterSizeOk = false;
	}
	OScInternal_NumRange_Destroy(clockWidthRange);
	OScInternal_NumRange *clockHeightRange = OScInternal_Device_GetRasterHeights(clockDevice);
	if (!OScInternal_NumRange_Contains(clockHeightRange, height)) {
		rasterSizeOk = false;
	}
	OScInternal_NumRange_Destroy(clockHeightRange);

	OSc_Device *detectorDevice = OSc_LSM_GetDetectorDevice(tmpl->lsm);
	OScInternal_NumRange *detectorWidthRange = OScInternal_Device_GetRasterWidths(detectorDevice);
	if (!OScInternal_NumRange_Contains(detectorWidthRange, width)) {
		rasterSizeOk = false;
	}
	OScInternal_NumRange_Destroy(detectorWidthRange);
	OScInternal_NumRange *detectorHeightRange = OScInternal_Device_GetRasterHeights(detectorDevice);
	if (!OScInternal_NumRange_Contains(detectorHeightRange, height)) {
		rasterSizeOk = false;
	}
	OScInternal_NumRange_Destroy(detectorHeightRange);

	if (!rasterSizeOk) {
		return OScInternal_Error_UnsupportedOperation();
	}

	tmpl->xOffset = xOffset;
	tmpl->yOffset = yOffset;
	tmpl->width = width;
	tmpl->height = height;
	return OSc_OK;
}


void OSc_AcqTemplate_ResetROI(OSc_AcqTemplate *tmpl)
{
	if (!tmpl)
		return;
	tmpl->xOffset = tmpl->yOffset = 0;
	tmpl->width = tmpl->height = tmpl->resolution;
}


OSc_RichError *OSc_AcqTemplate_GetROI(OSc_AcqTemplate *tmpl, uint32_t *xOffset, uint32_t *yOffset, uint32_t *width, uint32_t *height)
{
	if (!tmpl || !xOffset || !yOffset || !width || !height)
		return OScInternal_Error_IllegalArgument();
	*xOffset = tmpl->xOffset;
	*yOffset = tmpl->yOffset;
	*width = tmpl->width;
	*height = tmpl->height;
	return OSc_OK;
}


OSc_RichError *OSc_AcqTemplate_GetNumberOfChannels(OSc_AcqTemplate *tmpl, uint32_t *numberOfChannels)
{
	if (!tmpl || !numberOfChannels)
		return OScInternal_Error_IllegalArgument();

	// This implementation is temporary; device-specific settings that affect
	// the number of channels should belong to the AcqTemplate and we need a
	// mechanism to allow the device implementation to compute the number of
	// channels with the AcqTemplate settings as sole input.
	OSc_Device *detectorDevice = OSc_LSM_GetDetectorDevice(tmpl->lsm);
	return OScInternal_Device_GetNumberOfChannels(detectorDevice, numberOfChannels);
}


OSc_RichError *OSc_AcqTemplate_GetBytesPerSample(OSc_AcqTemplate *tmpl, uint32_t *bytesPerSample)
{
	if (!tmpl || !bytesPerSample)
		return OScInternal_Error_IllegalArgument();

	// This implementation is temporary; device-specific settings that affect
	// the sample format should belong to the AcqTemplate and we need a
	// mechanism to allow the device implementation to compute the sample
	// format with the AcqTemplate settings as sole input.
	OSc_Device *detectorDevice = OSc_LSM_GetDetectorDevice(tmpl->lsm);
	return OScInternal_Device_GetBytesPerSample(detectorDevice, bytesPerSample);
}
