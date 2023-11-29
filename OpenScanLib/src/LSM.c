#include "InternalErrors.h"
#include "OpenScanLibPrivate.h"

#include <ss8str.h>

#include <stdlib.h>

struct OScInternal_LSM {
    OSc_Device *clockDevice;
    OSc_Device *scannerDevice;
    OSc_Device *detectorDevice;

    // OSc_Device pointers opened for this LSM; no duplicates.
    OScInternal_PtrArray *associatedDevices;
};

OSc_RichError *OSc_LSM_Create(OSc_LSM **lsm) {
    *lsm = calloc(1, sizeof(OSc_LSM));
    (*lsm)->associatedDevices = OScInternal_PtrArray_Create();
    return OSc_OK;
}

OSc_RichError *OSc_LSM_Destroy(OSc_LSM *lsm) {
    if (!lsm)
        return OSc_OK;

    OSc_RichError *err;

    // We need to close each associated device, but doing so in turn
    // dissociates that device, so we need to make a copy of the list of
    // associated devices first.
    OScInternal_PtrArray *devicesToClose =
        OScInternal_PtrArray_Copy(lsm->associatedDevices);
    for (size_t i = 0; i < OScInternal_PtrArray_Size(devicesToClose); ++i) {
        OSc_Device *device = OScInternal_PtrArray_At(devicesToClose, i);
        if (OSc_CHECK_ERROR(err, OSc_Device_Close(device))) {
            ss8str msg;
            ss8_init_copy_cstr(&msg, "Error while closing device ");
            const char *name = NULL;
            OSc_Device_GetName(device, &name);
            ss8_cat_cstr(&msg, name ? name : "(unknown)");
            OScInternal_LogError(device, ss8_cstr(&msg));
            ss8_destroy(&msg);
        }
    }
    OScInternal_PtrArray_Destroy(devicesToClose);

    OScInternal_PtrArray_Destroy(lsm->associatedDevices);
    free(lsm);
    return OSc_OK;
}

OSc_Device *OSc_LSM_GetClockDevice(OSc_LSM *lsm) {
    if (!lsm)
        return NULL;
    return lsm->clockDevice;
}

OSc_Device *OSc_LSM_GetScannerDevice(OSc_LSM *lsm) {
    if (!lsm)
        return NULL;
    return lsm->scannerDevice;
}

OSc_Device *OSc_LSM_GetDetectorDevice(OSc_LSM *lsm) {
    if (!lsm)
        return NULL;
    return lsm->detectorDevice;
}

OSc_RichError *OSc_LSM_SetClockDevice(OSc_LSM *lsm, OSc_Device *clockDevice) {
    // TODO Should allow null device
    if (!lsm || !clockDevice)
        return OScInternal_Error_IllegalArgument();

    OSc_RichError *err;
    bool isAssociated = false;
    if (OSc_CHECK_ERROR(err, OScInternal_LSM_Is_Device_Associated(
                                 lsm, clockDevice, &isAssociated)))
        return err;
    if (!isAssociated)
        return OScInternal_Error_DeviceNotOpenedForLSM();

    lsm->clockDevice = clockDevice;
    return OSc_OK;
}

OSc_RichError *OSc_LSM_SetScannerDevice(OSc_LSM *lsm,
                                        OSc_Device *scannerDevice) {
    // TODO Should allow null device
    if (!lsm || !scannerDevice)
        return OScInternal_Error_IllegalArgument();

    OSc_RichError *err;
    bool isAssociated = false;
    if (OSc_CHECK_ERROR(err, OScInternal_LSM_Is_Device_Associated(
                                 lsm, scannerDevice, &isAssociated)))
        return err;
    if (!isAssociated)
        return OScInternal_Error_DeviceNotOpenedForLSM();

    lsm->scannerDevice = scannerDevice;
    return OSc_OK;
}

OSc_RichError *OSc_LSM_SetDetectorDevice(OSc_LSM *lsm,
                                         OSc_Device *detectorDevice) {
    // TODO Should allow null device
    if (!lsm || !detectorDevice)
        return OScInternal_Error_IllegalArgument();

    OSc_RichError *err;
    bool isAssociated = false;
    if (OSc_CHECK_ERROR(err, OScInternal_LSM_Is_Device_Associated(
                                 lsm, detectorDevice, &isAssociated)))
        return err;
    if (!isAssociated)
        return OScInternal_Error_DeviceNotOpenedForLSM();

    lsm->detectorDevice = detectorDevice;
    return OSc_OK;
}

OSc_RichError *OScInternal_LSM_Associate_Device(OSc_LSM *lsm,
                                                OSc_Device *device) {
    bool isAssociated = false;
    OSc_RichError *err;
    if (OSc_CHECK_ERROR(err, OScInternal_LSM_Is_Device_Associated(
                                 lsm, device, &isAssociated)))
        return err;
    if (isAssociated)
        return OScInternal_Error_DeviceAlreadyOpen();

    OScInternal_PtrArray_Append(lsm->associatedDevices, device);
    return OSc_OK;
}

OSc_RichError *OScInternal_LSM_Dissociate_Device(OSc_LSM *lsm,
                                                 OSc_Device *device) {
    for (size_t i = 0; i < OScInternal_PtrArray_Size(lsm->associatedDevices);
         ++i) {
        if (OScInternal_PtrArray_At(lsm->associatedDevices, i) == device) {
            OScInternal_PtrArray_Remove(lsm->associatedDevices, i);
            return OSc_OK;
        }
    }
    return OScInternal_Error_DeviceNotOpenedForLSM();
}

OSc_RichError *OScInternal_LSM_Is_Device_Associated(OSc_LSM *lsm,
                                                    OSc_Device *device,
                                                    bool *isAssociated) {
    *isAssociated = false;
    for (size_t i = 0; i < OScInternal_PtrArray_Size(lsm->associatedDevices);
         ++i) {
        if (OScInternal_PtrArray_At(lsm->associatedDevices, i) == device) {
            *isAssociated = true;
            break;
        }
    }

    return OSc_OK;
}

OSc_RichError *OSc_LSM_IsRunningAcquisition(OSc_LSM *lsm, bool *isRunning) {
    *isRunning = false;
    for (size_t i = 0; i < OScInternal_PtrArray_Size(lsm->associatedDevices);
         ++i) {
        OSc_Device *device =
            OScInternal_PtrArray_At(lsm->associatedDevices, i);
        OSc_RichError *err;
        if (OSc_CHECK_ERROR(err,
                            OScInternal_Device_IsRunning(device, isRunning)))
            return err;
        if (*isRunning)
            return OSc_OK;
    }
    return OSc_OK;
}
