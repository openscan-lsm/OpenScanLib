#include "InternalErrors.h"
#include "OpenScanLibPrivate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct OScInternal_Device {
    OScDev_ModuleImpl *modImpl;
    OScDev_DeviceImpl *impl;
    void *implData;

    OSc_LogFunc logFunc;
    void *logData;

    bool isOpen;

    OSc_LSM *associatedLSM;

    OScInternal_PtrArray *settings;

    char name[OSc_MAX_STR_LEN + 1];
    char displayName[OSc_MAX_STR_LEN + 1];
};

void OSc_Device_SetLogFunc(OSc_Device *device, OSc_LogFunc func, void *data) {
    if (!device)
        return;

    device->logFunc = func;
    device->logData = data;
}

OSc_RichError *OSc_Device_GetName(OSc_Device *device, const char **name) {
    OScDev_Error errCode;
    if (!strlen(device->name)) {
        errCode = device->impl->GetName(device, device->name);
        if (errCode)
            return OScInternal_Error_RetrieveFromDevice(device, errCode);
    }

    *name = device->name;
    return OSc_OK;
}

OSc_RichError *OSc_Device_GetDisplayName(OSc_Device *device,
                                         const char **name) {
    if (!strlen(device->displayName)) {
        OSc_RichError *err;
        OScDev_Error errCode;
        const char *modelName;
        errCode = device->impl->GetModelName(&modelName);
        if (errCode)
            return OScInternal_Error_RetrieveFromDevice(device, errCode);

        const char *deviceName;
        if (OSc_CHECK_ERROR(err, OSc_Device_GetName(device, &deviceName)))
            return err;
        snprintf(device->displayName, OSc_MAX_STR_LEN, "%s@%s", modelName,
                 deviceName);
    }

    *name = device->displayName;
    return OSc_OK;
}

OSc_RichError *OSc_Device_Open(OSc_Device *device, OSc_LSM *lsm) {
    if (device->isOpen) {
        if (device->associatedLSM == lsm)
            return OSc_OK;
        return OScInternal_Error_DeviceAlreadyOpen();
    }

    OSc_RichError *err;
    OScDev_Error errCode;
    errCode = device->impl->Open(device);
    if (errCode)
        return OScInternal_Error_RetrieveFromDevice(device, errCode);

    device->isOpen = true;

    if (OSc_CHECK_ERROR(err, OScInternal_LSM_Associate_Device(lsm, device)))
        goto Error;
    device->associatedLSM = lsm;

    return OSc_OK;

Error:
    OSc_Device_Close(device);
    return err;
}

OSc_RichError *OSc_Device_Close(OSc_Device *device) {
    if (!device || !device->isOpen)
        return OSc_OK;

    OSc_RichError *err;
    OScDev_Error errCode;
    if (device->associatedLSM) {
        if (OSc_CHECK_ERROR(err, OScInternal_LSM_Dissociate_Device(
                                     device->associatedLSM, device)))
            return err;
    }

    errCode = device->impl->Close(device);
    if (errCode)
        return OScInternal_Error_RetrieveFromDevice(device, errCode);

    device->isOpen = false;

    return OSc_OK;
}

OSc_RichError *OSc_Device_HasClock(OSc_Device *device, bool *hasClock) {
    *hasClock = false;

    OScDev_Error errCode = device->impl->HasClock(device, hasClock);
    return OScInternal_Error_RetrieveFromDevice(device, errCode);
}

OSc_RichError *OSc_Device_HasScanner(OSc_Device *device, bool *hasScanner) {
    *hasScanner = false;

    OScDev_Error errCode = device->impl->HasScanner(device, hasScanner);
    return OScInternal_Error_RetrieveFromDevice(device, errCode);
}

OSc_RichError *OSc_Device_HasDetector(OSc_Device *device, bool *hasDetector) {
    *hasDetector = false;

    OScDev_Error errCode = device->impl->HasDetector(device, hasDetector);
    return OScInternal_Error_RetrieveFromDevice(device, errCode);
}

OSc_RichError *OSc_Device_GetSettings(OSc_Device *device,
                                      OSc_Setting ***settings, size_t *count) {
    if (device->settings == NULL) {
        OScDev_Error errCode;
        errCode = device->impl->MakeSettings(device, &device->settings);
        if (errCode)
            return OScInternal_Error_RetrieveFromDevice(device, errCode);
    }

    *settings = (OScDev_Setting **)OScInternal_PtrArray_Data(device->settings);
    *count = OScInternal_PtrArray_Size(device->settings);
    return OSc_OK;
}

OScDev_Error OScInternal_Device_Create(OScDev_ModuleImpl *modImpl,
                                       OSc_Device **device,
                                       OScDev_DeviceImpl *impl, void *data) {
    *device = calloc(1, sizeof(OSc_Device));
    (*device)->modImpl = modImpl;
    (*device)->impl = impl;
    (*device)->implData = data;
    return OScDev_OK;
}

OSc_RichError *OScInternal_Device_Destroy(OSc_Device *device) {
    if (!device) {
        return OSc_OK;
    }

    device->impl->ReleaseInstance(device);

    if (device->settings) {
        size_t nSettings = OScInternal_PtrArray_Size(device->settings);
        for (size_t i = 0; i < nSettings; ++i) {
            OSc_Setting *setting =
                OScInternal_PtrArray_At(device->settings, i);
            OScInternal_Setting_Destroy(setting);
        }
        OScInternal_PtrArray_Destroy(device->settings);
    }

    free(device);
    return OSc_OK;
}

bool OScInternal_Device_Log(OSc_Device *device, OSc_LogLevel level,
                            const char *message) {
    if (!device || !device->logFunc)
        return false;
    device->logFunc(message, level, device->logData);
    return true;
}

void *OScInternal_Device_GetImplData(OSc_Device *device) {
    if (!device)
        return NULL;
    return device->implData;
}

OScDev_NumRange *OScInternal_Device_GetPixelRates(OSc_Device *device) {
    if (!device)
        return NULL;
    if (device->impl->GetPixelRates) {
        OScDev_NumRange *ret;
        OScDev_Error errCode;
        errCode = device->impl->GetPixelRates(device, &ret);
        if (errCode) {
            return OScInternal_NumRange_CreateDiscrete();
        }
        return ret;
    }
    return OScInternal_NumRange_CreateContinuous(1e-3, 1e10);
}

OScDev_NumRange *OScInternal_Device_GetResolutions(OSc_Device *device) {
    if (!device)
        return NULL;
    if (device->impl->GetResolutions) {
        OScDev_NumRange *ret;
        OScDev_Error errCode;
        errCode = device->impl->GetResolutions(device, &ret);
        if (errCode) {
            return OScInternal_NumRange_CreateDiscrete();
        }
        return ret;
    }
    return OScInternal_NumRange_CreateContinuous(1, INT32_MAX);
}

OScDev_NumRange *OScInternal_Device_GetZooms(OSc_Device *device) {
    if (!device)
        return NULL;
    if (device->impl->GetZoomFactors) {
        OScDev_NumRange *ret;
        OScDev_Error errCode;
        errCode = device->impl->GetZoomFactors(device, &ret);
        if (errCode) {
            return OScInternal_NumRange_CreateDiscrete();
        }
        return ret;
    }
    return OScInternal_NumRange_CreateContinuous(1e-6, 1e6);
}

bool OScInternal_Device_IsROIScanSupported(OSc_Device *device) {
    if (!device)
        return false;
    if (!device->impl->IsROIScanSupported) {
        return false;
    }

    bool supported;
    OScDev_Error errCode;
    errCode = device->impl->IsROIScanSupported(device, &supported);
    if (errCode) {
        return false;
    }
    return supported;
}

OScInternal_NumRange *OScInternal_Device_GetRasterWidths(OSc_Device *device) {
    if (!device)
        return NULL;
    if (device->impl->GetRasterWidths) {
        OScDev_NumRange *ret;
        OScDev_Error errCode = device->impl->GetRasterWidths(device, &ret);
        if (errCode) {
            return OScInternal_NumRange_CreateDiscrete();
        }
        return ret;
    }
    return OScInternal_NumRange_CreateContinuous(1, INT32_MAX);
}

OScInternal_NumRange *OScInternal_Device_GetRasterHeights(OSc_Device *device) {
    if (!device)
        return NULL;
    if (device->impl->GetRasterHeights) {
        OScDev_NumRange *ret;
        OScDev_Error errCode;
        errCode = device->impl->GetRasterHeights(device, &ret);
        if (errCode) {
            return OScInternal_NumRange_CreateDiscrete();
        }
        return ret;
    }
    return OScInternal_NumRange_CreateContinuous(1, INT32_MAX);
}

OSc_RichError *
OScInternal_Device_GetNumberOfChannels(OSc_Device *device,
                                       uint32_t *numberOfChannels) {
    if (!device || !numberOfChannels)
        return OScInternal_Error_IllegalArgument();
    if (device->impl->GetNumberOfChannels) {
        OScDev_Error errCode =
            device->impl->GetNumberOfChannels(device, numberOfChannels);
        return OScInternal_Error_RetrieveFromDevice(device, errCode);
    }
    *numberOfChannels = 0;
    return OScInternal_Error_DeviceDoesNotSupportDetector();
}

OSc_RichError *OScInternal_Device_GetBytesPerSample(OSc_Device *device,
                                                    uint32_t *bytesPerSample) {
    if (!device || !bytesPerSample)
        return OScInternal_Error_IllegalArgument();
    if (device->impl->GetBytesPerSample) {
        OScDev_Error errCode =
            device->impl->GetBytesPerSample(device, bytesPerSample);
        return OScInternal_Error_RetrieveFromDevice(device, errCode);
    }
    *bytesPerSample = 0;
    return OScInternal_Error_DeviceDoesNotSupportDetector();
}

OSc_RichError *OScInternal_Device_Arm(OSc_Device *device,
                                      OSc_Acquisition *acq) {
    if (!device || !acq)
        return OScInternal_Error_IllegalArgument();

    OScDev_Error errCode = device->impl->Arm(
        device, OScInternal_Acquisition_GetForDevice(acq, device));
    return OScInternal_Error_RetrieveFromDevice(device, errCode);
}

OSc_RichError *OScInternal_Device_Start(OSc_Device *device) {
    if (!device)
        return OScInternal_Error_IllegalArgument();

    OScDev_Error errCode = device->impl->Start(device);
    return OScInternal_Error_RetrieveFromDevice(device, errCode);
}

void OScInternal_Device_Stop(OSc_Device *device) {
    if (!device)
        return;

    device->impl->Stop(device);
}

void OScInternal_Device_Wait(OSc_Device *device) {
    if (!device)
        return;

    device->impl->Wait(device);
}

OSc_RichError *OScInternal_Device_IsRunning(OSc_Device *device,
                                            bool *isRunning) {
    if (!device || !isRunning)
        return OScInternal_Error_IllegalArgument();

    OScDev_Error errCode = device->impl->IsRunning(device, isRunning);
    return OScInternal_Error_RetrieveFromDevice(device, errCode);
}

bool OScInternal_Device_SupportsRichErrors(OSc_Device *device) {
    return device->modImpl->supportsRichErrors;
}
