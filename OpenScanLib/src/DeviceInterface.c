#include "DeviceInterface.h"
#include "OpenScanLibPrivate.h"

// This file contains thin wrappers around functions, to conform to the device
// interface. Please don't add any non-trivial logic here.

// TODO: Generalize to more than device; possibly accept impl ptrs
static void Log(OScDev_ModuleImpl *modImpl, OScDev_Device *device,
                OScDev_LogLevel dLevel, const char *message) {
    (void)modImpl;
    // This only works because we define the enums identically:
    OSc_LogLevel level = (OSc_LogLevel)dLevel;

    OScInternal_Log(device, level, message);
}

static OScDev_RichError *
Error_RegisterCodeDomain(OScDev_ModuleImpl *modImpl, const char *domainName,
                         OScDev_ErrorCodeFormat codeFormat) {
    (void)modImpl;
    return OScInternal_Error_RegisterCodeDomain(domainName, codeFormat);
}

static OScDev_Error Error_ReturnAsCode(OScDev_ModuleImpl *modImpl,
                                       OScDev_RichError *error) {
    (void)modImpl;
    return OScInternal_Error_ReturnAsCode(error);
}

static OScDev_RichError *Error_Create(OScDev_ModuleImpl *modImpl,
                                      const char *message) {
    (void)modImpl;
    return OScInternal_Error_Create(message);
}

static OScDev_RichError *Error_CreateWithCode(OScDev_ModuleImpl *modImpl,
                                              const char *domainName,
                                              OScDev_Error code,
                                              const char *message) {
    (void)modImpl;
    return OScInternal_Error_CreateWithCode(domainName, code, message);
}

static OScDev_RichError *Error_Wrap(OScDev_ModuleImpl *modImpl,
                                    OScDev_RichError *cause,
                                    const char *message) {
    (void)modImpl;
    return OScInternal_Error_Wrap(cause, message);
}

static OScDev_RichError *
Error_WrapWithCode(OScDev_ModuleImpl *modImpl, OSc_RichError *cause,
                   const char *domainName, int32_t code, const char *message) {
    (void)modImpl;
    return OScInternal_Error_WrapWithCode(cause, domainName, code, message);
}

static const char *Error_GetMessage(OScDev_ModuleImpl *modImpl,
                                    OSc_RichError *error) {
    (void)modImpl;
    return OScInternal_Error_GetMessage(error);
}

static const char *Error_GetDomain(OScDev_ModuleImpl *modImpl,
                                   OSc_RichError *error) {
    (void)modImpl;
    return OScInternal_Error_GetDomain(error);
}

static int32_t Error_GetCode(OScDev_ModuleImpl *modImpl,
                             OSc_RichError *error) {
    (void)modImpl;
    return OScInternal_Error_GetCode(error);
}

static OSc_RichError *Error_GetCause(OScDev_ModuleImpl *modImpl,
                                     OSc_RichError *error) {
    (void)modImpl;
    return OScInternal_Error_GetCause(error);
}

static void Error_Format(OScDev_ModuleImpl *modImpl, OSc_RichError *error,
                         char *buffer, size_t bufsize) {
    (void)modImpl;
    OScInternal_Error_Format(error, buffer, bufsize);
}

static void Error_FormatRecursive(OScDev_ModuleImpl *modImpl,
                                  OSc_RichError *error, char *buffer,
                                  size_t bufsize) {
    (void)modImpl;
    OScInternal_Error_FormatRecursive(error, buffer, bufsize);
}

static OSc_RichError *Error_AsRichError(OScDev_ModuleImpl *modImpl,
                                        OScDev_Error code) {
    (void)modImpl;
    return OScInternal_Error_AsRichError(code);
}

static void Error_Destroy(OScDev_ModuleImpl *modImpl, OSc_RichError *error) {
    (void)modImpl;
    OScInternal_Error_Destroy(error);
}

static OScInternal_PtrArray *PtrArray_Create(OScDev_ModuleImpl *modImpl) {
    (void)modImpl;
    return OScInternal_PtrArray_Create();
}

static OScInternal_PtrArray *
PtrArray_CreateFromNullTerminated(OScDev_ModuleImpl *modImpl,
                                  void *const *nullTerminatedArray) {
    (void)modImpl;
    return OScInternal_PtrArray_CreateFromNullTerminated(nullTerminatedArray);
}

static void PtrArray_Destroy(OScDev_ModuleImpl *modImpl,
                             const OScInternal_PtrArray *arr) {
    (void)modImpl;
    OScInternal_PtrArray_Destroy(arr);
}

static void PtrArray_Append(OScDev_ModuleImpl *modImpl,
                            OScInternal_PtrArray *arr, void *obj) {
    (void)modImpl;
    OScInternal_PtrArray_Append(arr, obj);
}

static size_t PtrArray_Size(OScDev_ModuleImpl *modImpl,
                            const OScInternal_PtrArray *arr) {
    (void)modImpl;
    return OScInternal_PtrArray_Size(arr);
}

static bool PtrArray_Empty(OScDev_ModuleImpl *modImpl,
                           const OScInternal_PtrArray *arr) {
    (void)modImpl;
    return OScInternal_PtrArray_Empty(arr);
}

static void *PtrArray_At(OScDev_ModuleImpl *modImpl,
                         const OScInternal_PtrArray *arr, size_t index) {
    (void)modImpl;
    return OScInternal_PtrArray_At(arr, index);
}

static OScDev_NumArray *NumArray_Create(OScDev_ModuleImpl *modImpl) {
    (void)modImpl;
    return OScInternal_NumArray_Create();
}

static OScDev_NumArray *
NumArray_CreateFromNaNTerminated(OScDev_ModuleImpl *modImpl,
                                 const double *nanTerminatedArray) {
    (void)modImpl;
    return OScInternal_NumArray_CreateFromNaNTerminated(nanTerminatedArray);
}

static void NumArray_Destroy(OScDev_ModuleImpl *modImpl,
                             const OScDev_NumArray *arr) {
    (void)modImpl;
    OScInternal_NumArray_Destroy(arr);
}

static void NumArray_Append(OScDev_ModuleImpl *modImpl, OScDev_NumArray *arr,
                            double val) {
    (void)modImpl;
    OScInternal_NumArray_Append(arr, val);
}

static size_t NumArray_Size(OScDev_ModuleImpl *modImpl,
                            const OScDev_NumArray *arr) {
    (void)modImpl;
    return OScInternal_NumArray_Size(arr);
}

static bool NumArray_Empty(OScDev_ModuleImpl *modImpl,
                           const OScDev_NumArray *arr) {
    (void)modImpl;
    return OScInternal_NumArray_Empty(arr);
}

static double NumArray_At(OScDev_ModuleImpl *modImpl,
                          const OScDev_NumArray *arr, size_t index) {
    (void)modImpl;
    return OScInternal_NumArray_At(arr, index);
}

static OScDev_NumRange *NumRange_CreateContinuous(OScDev_ModuleImpl *modImpl,
                                                  double rMin, double rMax) {
    (void)modImpl;
    return OScInternal_NumRange_CreateContinuous(rMin, rMax);
}

static OScDev_NumRange *NumRange_CreateDiscrete(OScDev_ModuleImpl *modImpl) {
    (void)modImpl;
    return OScInternal_NumRange_CreateDiscrete();
}

static OScDev_NumRange *
NumRange_CreateDiscreteFromNaNTerminated(OScDev_ModuleImpl *modImpl,
                                         const double *nanTerminatedArray) {
    (void)modImpl;
    return OScInternal_NumRange_CreateDiscreteFromNaNTerminated(
        nanTerminatedArray);
}

static void NumRange_Destroy(OScDev_ModuleImpl *modImpl,
                             const OScDev_NumRange *range) {
    (void)modImpl;
    OScInternal_NumRange_Destroy(range);
}

static void NumRange_AppendDiscrete(OScDev_ModuleImpl *modImpl,
                                    OScDev_NumRange *range, double val) {
    (void)modImpl;
    OScInternal_NumRange_AppendDiscrete(range, val);
}

// TODO: Replace with direct provision of impl and data (once we have
// DeviceLoaders)
static OScDev_Error Device_Create(OScDev_ModuleImpl *modImpl,
                                  OScDev_Device **device,
                                  OScDev_DeviceImpl *impl, void *data) {
    return OScInternal_Device_Create(modImpl, device, impl, data);
}

static void *Device_GetImplData(OScDev_ModuleImpl *modImpl,
                                OScDev_Device *device) {
    (void)modImpl;
    return OScInternal_Device_GetImplData(device);
}

// TODO: Replace with a SettingsCreateContext-based method
static OScDev_Error Setting_Create(OScDev_ModuleImpl *modImpl,
                                   OScDev_Setting **setting, const char *name,
                                   OScDev_ValueType valueType,
                                   OScDev_SettingImpl *impl, void *data) {
    return OScInternal_Error_ReturnAsCode(OScInternal_Setting_Create(
        modImpl, setting, name, valueType, impl, data));
}

static void Setting_Destroy(OScDev_ModuleImpl *modImpl,
                            OScDev_Setting *setting) {
    (void)modImpl;
    OScInternal_Setting_Destroy(setting);
}

static void *Setting_GetImplData(OScDev_ModuleImpl *modImpl,
                                 OScDev_Setting *setting) {
    (void)modImpl;
    return OScInternal_Setting_GetImplData(setting);
}

static OScDev_Error Acquisition_IsClockRequested(OScDev_ModuleImpl *modImpl,
                                                 OScDev_Acquisition *devAcq,
                                                 bool *isRequested) {
    (void)modImpl;
    OSc_Device *device = OScInternal_AcquisitionForDevice_GetDevice(devAcq);
    OSc_Acquisition *acq =
        OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
    OSc_Device *clockDevice = OScInternal_Acquisition_GetClockDevice(acq);
    *isRequested = device == clockDevice;
    return OScDev_OK;
}

static OScDev_Error Acquisition_IsScannerRequested(OScDev_ModuleImpl *modImpl,
                                                   OScDev_Acquisition *devAcq,
                                                   bool *isRequested) {
    (void)modImpl;
    OSc_Device *device = OScInternal_AcquisitionForDevice_GetDevice(devAcq);
    OSc_Acquisition *acq =
        OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
    OSc_Device *scannerDevice = OScInternal_Acquisition_GetScannerDevice(acq);
    *isRequested = device == scannerDevice;
    return OScDev_OK;
}

static OScDev_Error Acquisition_IsDetectorRequested(OScDev_ModuleImpl *modImpl,
                                                    OScDev_Acquisition *devAcq,
                                                    bool *isRequested) {
    (void)modImpl;
    *isRequested = false;
    OSc_Device *device = OScInternal_AcquisitionForDevice_GetDevice(devAcq);
    OSc_Acquisition *acq =
        OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
    for (size_t i = 0;
         i < OScInternal_Acquisition_GetNumberOfDetectorDevices(acq); ++i) {
        if (device == OScInternal_Acquisition_GetDetectorDevice(acq, i)) {
            *isRequested = true;
            return OScDev_OK;
        }
    }
    return OScDev_OK;
}

static OScDev_Error
Acquisition_GetClockStartTriggerSource(OScDev_ModuleImpl *modImpl,
                                       OScDev_Acquisition *devAcq,
                                       OScDev_TriggerSource *startTrigger) {
    (void)modImpl;
    (void)devAcq;
    // At this moment we don't yet support external start trigger.
    *startTrigger = OScDev_TriggerSource_Software;
    return OScDev_OK;
}

static OScDev_Error Acquisition_GetClockSource(OScDev_ModuleImpl *modImpl,
                                               OScDev_Acquisition *devAcq,
                                               OScDev_ClockSource *clock) {
    (void)modImpl;
    OSc_Device *device = OScInternal_AcquisitionForDevice_GetDevice(devAcq);
    OSc_Acquisition *acq =
        OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
    OSc_Device *clockDevice = OScInternal_Acquisition_GetClockDevice(acq);

    if (device == clockDevice)
        *clock = OScDev_ClockSource_Internal;
    else
        *clock = OScDev_ClockSource_External;
    return OScDev_OK;
}

static uint32_t Acquisition_GetNumberOfFrames(OScDev_ModuleImpl *modImpl,
                                              OScDev_Acquisition *devAcq) {
    (void)modImpl;
    OSc_Acquisition *acq =
        OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
    return OScInternal_Acquisition_GetNumberOfFrames(acq);
}

static double Acquisition_GetPixelRate(OScDev_ModuleImpl *modImpl,
                                       OScDev_Acquisition *devAcq) {
    (void)modImpl;
    OSc_Acquisition *acq =
        OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
    return OSc_Acquisition_GetPixelRate(acq);
}

static uint32_t Acquisition_GetResolution(OScDev_ModuleImpl *modImpl,
                                          OScDev_Acquisition *devAcq) {
    (void)modImpl;
    OSc_Acquisition *acq =
        OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
    return OSc_Acquisition_GetResolution(acq);
}

static double Acquisition_GetZoomFactor(OScDev_ModuleImpl *modImpl,
                                        OScDev_Acquisition *devAcq) {
    (void)modImpl;
    OSc_Acquisition *acq =
        OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
    return OSc_Acquisition_GetZoomFactor(acq);
}

static void Acquisition_GetROI(OScDev_ModuleImpl *modImpl,
                               OScDev_Acquisition *devAcq, uint32_t *xOffset,
                               uint32_t *yOffset, uint32_t *width,
                               uint32_t *height) {
    (void)modImpl;
    OSc_Acquisition *acq =
        OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
    OSc_Acquisition_GetROI(acq, xOffset, yOffset, width, height);
}

static bool Acquisition_CallFrameCallback(OScDev_ModuleImpl *modImpl,
                                          OScDev_Acquisition *devAcq,
                                          uint32_t channel, void *pixels) {
    (void)modImpl;
    size_t detIdx =
        OScInternal_AcquisitionForDevice_GetDetectorDeviceIndex(devAcq);
    OSc_Acquisition *acq =
        OScInternal_AcquisitionForDevice_GetAcquisition(devAcq);
    return OScInternal_Acquisition_CallFrameCallback(acq, detIdx, channel,
                                                     pixels);
}

struct OScDevInternal_Interface DeviceInterfaceFunctionTable = {
    .Log = Log,
    .Error_RegisterCodeDomain = Error_RegisterCodeDomain,
    .Error_ReturnAsCode = Error_ReturnAsCode,
    .Error_Create = Error_Create,
    .Error_CreateWithCode = Error_CreateWithCode,
    .Error_Wrap = Error_Wrap,
    .Error_WrapWithCode = Error_WrapWithCode,
    .Error_GetMessage = Error_GetMessage,
    .Error_GetDomain = Error_GetDomain,
    .Error_GetCode = Error_GetCode,
    .Error_GetCause = Error_GetCause,
    .Error_Destroy = Error_Destroy,
    .Error_Format = Error_Format,
    .Error_FormatRecursive = Error_FormatRecursive,
    .Error_AsRichError = Error_AsRichError,
    .PtrArray_Create = PtrArray_Create,
    .PtrArray_CreateFromNullTerminated = PtrArray_CreateFromNullTerminated,
    .PtrArray_Destroy = PtrArray_Destroy,
    .PtrArray_Append = PtrArray_Append,
    .PtrArray_Size = PtrArray_Size,
    .PtrArray_Empty = PtrArray_Empty,
    .PtrArray_At = PtrArray_At,
    .NumArray_Create = NumArray_Create,
    .NumArray_CreateFromNaNTerminated = NumArray_CreateFromNaNTerminated,
    .NumArray_Destroy = NumArray_Destroy,
    .NumArray_Append = NumArray_Append,
    .NumArray_Size = NumArray_Size,
    .NumArray_Empty = NumArray_Empty,
    .NumArray_At = NumArray_At,
    .NumRange_CreateContinuous = NumRange_CreateContinuous,
    .NumRange_CreateDiscrete = NumRange_CreateDiscrete,
    .NumRange_CreateDiscreteFromNaNTerminated =
        NumRange_CreateDiscreteFromNaNTerminated,
    .NumRange_Destroy = NumRange_Destroy,
    .NumRange_AppendDiscrete = NumRange_AppendDiscrete,
    .Device_Create = Device_Create,
    .Device_GetImplData = Device_GetImplData,
    .Setting_Create = Setting_Create,
    .Setting_GetImplData = Setting_GetImplData,
    .Acquisition_IsClockRequested = Acquisition_IsClockRequested,
    .Acquisition_IsScannerRequested = Acquisition_IsScannerRequested,
    .Acquisition_IsDetectorRequested = Acquisition_IsDetectorRequested,
    .Acquisition_GetClockStartTriggerSource =
        Acquisition_GetClockStartTriggerSource,
    .Acquisition_GetClockSource = Acquisition_GetClockSource,
    .Acquisition_GetNumberOfFrames = Acquisition_GetNumberOfFrames,
    .Acquisition_GetPixelRate = Acquisition_GetPixelRate,
    .Acquisition_GetResolution = Acquisition_GetResolution,
    .Acquisition_GetZoomFactor = Acquisition_GetZoomFactor,
    .Acquisition_GetROI = Acquisition_GetROI,
    .Acquisition_CallFrameCallback = Acquisition_CallFrameCallback,
};
