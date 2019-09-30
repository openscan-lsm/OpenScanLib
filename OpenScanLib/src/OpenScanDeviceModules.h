#pragma once

#include "OpenScanLibPrivate.h"


OSc_Error OSc_DeviceModule_Get_Count(size_t *count);
OSc_Error OSc_DeviceModule_Get_Names(const char **modules, size_t *count);
OSc_Error OSc_DeviceModule_GetDeviceImpls(const char *module, const OScDev_PtrArray **deviceImpls);