#pragma once

#include "OpenScanLibPrivate.h"

OSc_RichError *OScInternal_Error_WrongConstraintType();

OSc_RichError *OScInternal_Error_IllegalArgument();

OSc_RichError *OScInternal_Error_Unknown();

OSc_RichError *OScInternal_Error_OutOfMemory();

OSc_RichError *OScInternal_Error_UnsupportedOperation();

OSc_RichError *OScInternal_Error_OutOfRange();

OSc_RichError *OScInternal_Error_EmptyRaster();

OSc_RichError *OScInternal_Error_DeviceAlreadyOpen();

OSc_RichError *OScInternal_Error_DeviceAlreadyInUseAsDetector();

OSc_RichError *OScInternal_Error_DeviceDoesNotSupportDetector();

OSc_RichError *OScInternal_Error_TooManyDetectorDevices();

OSc_RichError *OScInternal_Error_NoDetectorDeviceEnabled();

OSc_RichError *OScInternal_Error_NoDetectorChannelEnabled();

OSc_RichError *OScInternal_Error_NonUniformBytesPerSample();

OSc_RichError *OScInternal_Error_DeviceModuleAlreadyExists();

OSc_RichError *OScInternal_Error_NoSuchDeviceModule();

OSc_RichError *OScInternal_Error_DeviceNotOpenedForLSM();
