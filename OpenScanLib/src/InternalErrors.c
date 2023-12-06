#include "OpenScanLibPrivate.h"

// Internal errors
OSc_RichError *OScInternal_Error_WrongConstraintType() {
    return OScInternal_Error_Create("Wrong constraint type");
}

OSc_RichError *OScInternal_Error_IllegalArgument() {
    return OScInternal_Error_Create("Illegal argument");
}

OSc_RichError *OScInternal_Error_Unknown() {
    return OScInternal_Error_Create("Error unknown");
}

OSc_RichError *OScInternal_Error_OutOfMemory() {
    return OScInternal_Error_Create("Out of memory");
}

OSc_RichError *OScInternal_Error_UnsupportedOperation() {
    return OScInternal_Error_Create("Unsupported operation");
}

OSc_RichError *OScInternal_Error_OutOfRange() {
    return OScInternal_Error_Create("Out of range");
}

OSc_RichError *OScInternal_Error_EmptyRaster() {
    return OScInternal_Error_Create("Empty raster");
}

OSc_RichError *OScInternal_Error_DeviceAlreadyOpen() {
    return OScInternal_Error_Create("Device already open");
}

OSc_RichError *OScInternal_Error_DeviceAlreadyInUseAsDetector() {
    return OScInternal_Error_Create("Device already in use as detector");
}

OSc_RichError *OScInternal_Error_DeviceDoesNotSupportDetector() {
    return OScInternal_Error_Create("Device does not support detector");
}

OSc_RichError *OScInternal_Error_TooManyDetectorDevices() {
    return OScInternal_Error_Create(
        "Number of allowed detector devices has been exceeded");
}

OSc_RichError *OScInternal_Error_NoDetectorDeviceEnabled() {
    return OScInternal_Error_Create("No detector device enabled");
}

OSc_RichError *OScInternal_Error_NoDetectorChannelEnabled() {
    return OScInternal_Error_Create("No channel in any detector enabled");
}

OSc_RichError *OScInternal_Error_NonUniformBytesPerSample() {
    return OScInternal_Error_Create(
        "Differing bytes-per-sample among enabled detector devices");
}

OSc_RichError *OScInternal_Error_DeviceModuleAlreadyExists() {
    return OScInternal_Error_Create("Device module already exists");
}

OSc_RichError *OScInternal_Error_NoSuchDeviceModule() {
    return OScInternal_Error_Create("No such device module");
}

OSc_RichError *OScInternal_Error_DeviceNotOpenedForLSM() {
    return OScInternal_Error_Create("Device not opened for LSM");
}
