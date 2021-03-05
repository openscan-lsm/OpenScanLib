#pragma once

#include "OScNIDAQDevicePrivate.h"

#define NUM_SLOTS_IN_CHASSIS 4
#define MAX_NUM_PORTS 256



OScDev_Error NIDAQEnumerateInstances(OScDev_PtrArray **devices);
OScDev_Error ParseDeviceNameList(char *names,
	char(*deviceNames)[OScDev_MAX_STR_LEN + 1], size_t *deviceCount);
OScDev_Error OpenDAQ(OScDev_Device *device);
OScDev_Error CloseDAQ(OScDev_Device *device);
OScDev_Error GetTerminalNameWithDevPrefix(TaskHandle taskHandle, const char terminalName[], char triggerName[]);
OScDev_Error ReconfigDAQ(OScDev_Device *device, OScDev_Acquisition *acq);
OScDev_Error RunAcquisitionLoop(OScDev_Device *device);
OScDev_Error StopAcquisitionAndWait(OScDev_Device *device);
OScDev_Error IsAcquisitionRunning(OScDev_Device *device, bool *isRunning);
OScDev_Error WaitForAcquisitionToFinish(OScDev_Device *device);
