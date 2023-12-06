#include "InternalErrors.h"
#include "OpenScanLibPrivate.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

struct OScInternal_AcquisitionForDevice {
    OSc_Device *device;
    OSc_Acquisition *acq;
    size_t detectorIndex; // SIZE_MAX if not detector
};

struct OScInternal_Acquisition {
    OSc_Device *clockDevice;
    OSc_Device *scannerDevice;
    OScInternal_PtrArray *detectorDevices;
    OSc_FrameCallback frameCallback;
    void *data;

    uint32_t numberOfFrames;
    double pixelRateHz;
    int32_t resolution;
    double zoomFactor;
    uint32_t xOffset;
    uint32_t yOffset;
    uint32_t width;
    uint32_t height;

    // Global channel number of first device-local channel, indexed by detector
    // device.
    OScInternal_NumArray *channelOffsets;

    uint32_t numberOfChannels;
    uint32_t bytesPerSample;

    // We can pass opaque pointers to these structs to devices, so that we can
    // handle acquisition-related calls in a device-specific manner.
    struct OScInternal_AcquisitionForDevice acqForClockDevice;
    struct OScInternal_AcquisitionForDevice acqForScannerDevice;
    // Owns elements of type struct OScInternal_AcquisitionForDevice, with
    // detectorIndex set to 0, 1, 2,... for reverse lookup:
    OScInternal_PtrArray *acqsForDetectorDevices;
};

OSc_Device *
OScInternal_AcquisitionForDevice_GetDevice(OScDev_Acquisition *devAcq) {
    return devAcq->device;
}

size_t OScInternal_AcquisitionForDevice_GetDetectorDeviceIndex(
    OScDev_Acquisition *devAcq) {
    assert(devAcq->detectorIndex != -1);
    return devAcq->detectorIndex;
}

OSc_Acquisition *
OScInternal_AcquisitionForDevice_GetAcquisition(OScDev_Acquisition *devAcq) {
    return devAcq->acq;
}

OSc_RichError *OSc_Acquisition_Create(OSc_Acquisition **acq,
                                      OSc_AcqTemplate *tmpl) {
    OSc_LSM *lsm = OSc_AcqTemplate_GetLSM(tmpl);

    uint32_t nChans;
    OSc_RichError *err = OSc_AcqTemplate_GetNumberOfChannels(tmpl, &nChans);
    if (err != OSc_OK)
        return err;
    if (nChans == 0)
        return OScInternal_Error_NoDetectorChannelEnabled();

    uint32_t bytesPerSamp;
    err = OSc_AcqTemplate_GetBytesPerSample(tmpl, &bytesPerSamp);
    if (err != OSc_OK)
        return err;

    *acq = calloc(1, sizeof(OSc_Acquisition));

    (*acq)->clockDevice = OSc_LSM_GetClockDevice(lsm);
    (*acq)->scannerDevice = OSc_LSM_GetScannerDevice(lsm);
    (*acq)->detectorDevices = OScInternal_PtrArray_Create();
    (*acq)->channelOffsets = OScInternal_NumArray_Create();
    uint32_t nChansSoFar = 0;
    for (size_t i = 0; i < OSc_LSM_GetNumberOfDetectorDevices(lsm); ++i) {
        if (OSc_AcqTemplate_IsDetectorDeviceEnabled(tmpl, i)) {
            // Only add detector device to acquisition if >0 channels enabled.
            OSc_Device *detectorDevice = OSc_LSM_GetDetectorDevice(lsm, i);
            uint32_t nch;
            err = OScInternal_Device_GetNumberOfChannels(detectorDevice, &nch);
            assert(err == OSc_OK); // Given earlier GetNumberOfChannels
            if (nch >= 0) {
                OScInternal_PtrArray_Append((*acq)->detectorDevices,
                                            detectorDevice);
                OScInternal_NumArray_Append((*acq)->channelOffsets,
                                            nChansSoFar);
                nChansSoFar += nch;
            }
        }
    }

    (*acq)->numberOfFrames = OSc_AcqTemplate_GetNumberOfFrames(tmpl);
    OSc_Setting *setting;
    OSc_AcqTemplate_GetPixelRateSetting(tmpl, &setting);
    OSc_Setting_GetFloat64Value(setting, &(*acq)->pixelRateHz);
    OSc_AcqTemplate_GetResolutionSetting(tmpl, &setting);
    OSc_Setting_GetInt32Value(setting, &(*acq)->resolution);
    OSc_AcqTemplate_GetZoomFactorSetting(tmpl, &setting);
    OSc_Setting_GetFloat64Value(setting, &(*acq)->zoomFactor);
    OSc_AcqTemplate_GetROI(tmpl, &(*acq)->xOffset, &(*acq)->yOffset,
                           &(*acq)->width, &(*acq)->height);

    (*acq)->numberOfChannels = nChans;
    (*acq)->bytesPerSample = bytesPerSamp;

    (*acq)->acqForClockDevice.device = (*acq)->clockDevice;
    (*acq)->acqForClockDevice.acq = *acq;
    (*acq)->acqForClockDevice.detectorIndex = SIZE_MAX;
    (*acq)->acqForScannerDevice.device = (*acq)->scannerDevice;
    (*acq)->acqForScannerDevice.acq = *acq;
    (*acq)->acqForScannerDevice.detectorIndex = SIZE_MAX;
    (*acq)->acqsForDetectorDevices = OScInternal_PtrArray_Create();
    for (size_t i = 0; i < OScInternal_PtrArray_Size((*acq)->detectorDevices);
         ++i) {
        struct OScInternal_AcquisitionForDevice *afd =
            malloc(sizeof(struct OScInternal_AcquisitionForDevice));
        afd->device = OScInternal_PtrArray_At((*acq)->detectorDevices, i);
        afd->acq = *acq;
        afd->detectorIndex = i;
        OScInternal_PtrArray_Append((*acq)->acqsForDetectorDevices, afd);
    }

    return OSc_OK;
}

OSc_RichError *OSc_Acquisition_Destroy(OSc_Acquisition *acq) {
    for (size_t i = 0;
         i < OScInternal_PtrArray_Size(acq->acqsForDetectorDevices); ++i)
        free(OScInternal_PtrArray_At(acq->acqsForDetectorDevices, i));
    OScInternal_PtrArray_Destroy(acq->acqsForDetectorDevices);
    OScInternal_NumArray_Destroy(acq->channelOffsets);
    OScInternal_PtrArray_Destroy(acq->detectorDevices);
    free(acq);
    return OSc_OK;
}

OSc_RichError *OSc_Acquisition_SetNumberOfFrames(OSc_Acquisition *acq,
                                                 uint32_t numberOfFrames) {
    acq->numberOfFrames = numberOfFrames;
    return OSc_OK;
}

OSc_RichError *OSc_Acquisition_SetFrameCallback(OSc_Acquisition *acq,
                                                OSc_FrameCallback callback) {
    acq->frameCallback = callback;
    return OSc_OK;
}

OSc_RichError *OSc_Acquisition_GetData(OSc_Acquisition *acq, void **data) {
    *data = acq->data;
    return OSc_OK;
}

OSc_RichError *OSc_Acquisition_SetData(OSc_Acquisition *acq, void *data) {
    acq->data = data;
    return OSc_OK;
}

uint32_t OSc_Acquisition_GetNumberOfFrames(OSc_Acquisition *acq) {
    if (!acq)
        return 0;
    return acq->numberOfFrames;
}

double OSc_Acquisition_GetPixelRate(OSc_Acquisition *acq) {
    if (!acq)
        return NAN;
    return acq->pixelRateHz;
}

uint32_t OSc_Acquisition_GetResolution(OSc_Acquisition *acq) {
    if (!acq)
        return 0;
    return acq->resolution;
}

double OSc_Acquisition_GetZoomFactor(OSc_Acquisition *acq) {
    if (!acq)
        return NAN;
    return acq->zoomFactor;
}

void OSc_Acquisition_GetROI(OSc_Acquisition *acq, uint32_t *xOffset,
                            uint32_t *yOffset, uint32_t *width,
                            uint32_t *height) {
    if (!acq) {
        *xOffset = *yOffset = 0;
        *width = *height = 0;
        return;
    }
    if (xOffset)
        *xOffset = acq->xOffset;
    if (yOffset)
        *yOffset = acq->yOffset;
    if (width)
        *width = acq->width;
    if (height)
        *height = acq->height;
}

OSc_RichError *
OSc_Acquisition_GetNumberOfChannels(OSc_Acquisition *acq,
                                    uint32_t *numberOfChannels) {
    if (!acq || !numberOfChannels)
        return OScInternal_Error_IllegalArgument();
    *numberOfChannels = acq->numberOfChannels;
    return OSc_OK;
}

OSc_RichError *OSc_Acquisition_GetBytesPerSample(OSc_Acquisition *acq,
                                                 uint32_t *bytesPerSample) {
    if (!acq || !bytesPerSample)
        return OScInternal_Error_IllegalArgument();
    *bytesPerSample = acq->bytesPerSample;
    return OSc_OK;
}

OSc_RichError *OSc_Acquisition_Arm(OSc_Acquisition *acq) {
    // Arm each device participating in the acquisition exactly once each

    OSc_RichError *err;

    // Clock
    if (OSc_CHECK_ERROR(err, OScInternal_Device_Arm(acq->clockDevice, acq)))
        return err;

    // Scanner, if different device
    if (acq->scannerDevice != acq->clockDevice) {
        if (OSc_CHECK_ERROR(err,
                            OScInternal_Device_Arm(acq->scannerDevice, acq))) {
            OScInternal_Device_Stop(acq->clockDevice);
            return err;
        }
    }

    // Detectors, if different from clock/scanner
    for (size_t i = 0; i < OScInternal_PtrArray_Size(acq->detectorDevices);
         ++i) {
        OSc_Device *detectorDevice =
            OScInternal_PtrArray_At(acq->detectorDevices, i);
        if (detectorDevice != acq->clockDevice &&
            detectorDevice != acq->scannerDevice) {
            if (OSc_CHECK_ERROR(err,
                                OScInternal_Device_Arm(detectorDevice, acq))) {
                for (size_t j = i; j > 0; --j) {
                    OScInternal_Device_Stop(
                        OScInternal_PtrArray_At(acq->detectorDevices, j - 1));
                }
                OScInternal_Device_Stop(acq->scannerDevice);
                if (acq->clockDevice != acq->scannerDevice)
                    OScInternal_Device_Stop(acq->clockDevice);
                return err;
            }
        }
    }

    return OSc_OK;
}

OSc_RichError *OSc_Acquisition_Start(OSc_Acquisition *acq) {
    // TODO Error if not armed
    return OScInternal_Device_Start(acq->clockDevice);
}

OSc_RichError *OSc_Acquisition_Stop(OSc_Acquisition *acq) {
    // Stop() is idempotent, so we don't bother to determine the unique devices
    OScInternal_Device_Stop(acq->clockDevice);
    OScInternal_Device_Stop(acq->scannerDevice);
    for (size_t i = 0; i < OScInternal_PtrArray_Size(acq->detectorDevices);
         ++i) {
        OScInternal_Device_Stop(
            OScInternal_PtrArray_At(acq->detectorDevices, i));
    }

    return OSc_OK;
}

OSc_RichError *OSc_Acquisition_Wait(OSc_Acquisition *acq) {
    OScInternal_Device_Wait(acq->clockDevice);
    OScInternal_Device_Wait(acq->scannerDevice);
    for (size_t i = 0; i < OScInternal_PtrArray_Size(acq->detectorDevices);
         ++i) {
        OScInternal_Device_Wait(
            OScInternal_PtrArray_At(acq->detectorDevices, i));
    }
    return OSc_OK;
}

uint32_t OScInternal_Acquisition_GetNumberOfFrames(OSc_Acquisition *acq) {
    return acq->numberOfFrames;
}

OSc_Device *OScInternal_Acquisition_GetClockDevice(OSc_Acquisition *acq) {
    return acq->clockDevice;
}

OSc_Device *OScInternal_Acquisition_GetScannerDevice(OSc_Acquisition *acq) {
    return acq->scannerDevice;
}

size_t
OScInternal_Acquisition_GetNumberOfDetectorDevices(OSc_Acquisition *acq) {
    return OScInternal_PtrArray_Size(acq->detectorDevices);
}

OSc_Device *OScInternal_Acquisition_GetDetectorDevice(OSc_Acquisition *acq,
                                                      size_t index) {
    return OScInternal_PtrArray_At(acq->detectorDevices, index);
}

OScDev_Acquisition *OScInternal_Acquisition_GetForDevice(OSc_Acquisition *acq,
                                                         OSc_Device *device) {
    // We need to search the detectors first, because the clock/scanner may
    // duplicate the AcquisitionForDevice but would lack the detector index.
    for (size_t i = 0; i < OScInternal_PtrArray_Size(acq->detectorDevices);
         ++i) {
        if (OScInternal_PtrArray_At(acq->detectorDevices, i) == device) {
            return OScInternal_PtrArray_At(acq->acqsForDetectorDevices, i);
        }
    }

    if (acq->clockDevice == device)
        return &acq->acqForClockDevice;
    if (acq->scannerDevice == device)
        return &acq->acqForScannerDevice;
    return NULL;
}

bool OScInternal_Acquisition_CallFrameCallback(OSc_Acquisition *acq,
                                               size_t detectorIndex,
                                               uint32_t channel,
                                               void *pixels) {
    if (acq->frameCallback == NULL)
        return true;

    uint32_t chanOffset =
        (uint32_t)OScInternal_NumArray_At(acq->channelOffsets, detectorIndex);
    return acq->frameCallback(acq, chanOffset + channel, pixels, acq->data);
}
