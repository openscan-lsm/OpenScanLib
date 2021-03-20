#include "OpenScanDeviceLib.h"

#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <stdio.h>
#include <time.h>


uint16_t* buf_frame;
static OScDev_DeviceImpl g_SimulatedDeviceImpl;


struct DevicePrivateData
{
	bool simulatedErrorOnStart;

	struct
	{
		CRITICAL_SECTION mutex;
		HANDLE thread;
		CONDITION_VARIABLE acquisitionFinishCondition;
		bool running;
		bool armed; // Valid when running == true
		bool started; // Valid when running == true
		bool stopRequested; // Valid when running == true
		OScDev_Acquisition* acquisition;
	} acquisition;
};


static inline struct DevicePrivateData* GetSettingDeviceData(OScDev_Setting* setting)
{
	return (struct OScNIDAQPrivateData*)OScDev_Device_GetImplData((OScDev_Device*)OScDev_Setting_GetImplData(setting));
}


static OScDev_Error GetErrorOnStart(OScDev_Setting* setting, bool* value)
{
	*value = GetSettingDeviceData(setting)->simulatedErrorOnStart;

	return OScDev_OK;
}


static OScDev_Error SetErrorOnStart(OScDev_Setting* setting, bool value)
{
	GetSettingDeviceData(setting)->simulatedErrorOnStart = value;
	return OScDev_OK;
}


OScDev_SettingImpl SettingImpl_ErrorOnStart = {
	.GetBool = GetErrorOnStart,
	.SetBool = SetErrorOnStart,
};


static inline struct DevicePrivateData* GetData(OScDev_Device* device)
{
	return (struct DevicePrivateData*)OScDev_Device_GetImplData(device);
}


static void InitializeDevicePrivateData(struct DevicePrivateData* data)
{

	data->simulatedErrorOnStart = false;

	InitializeCriticalSection(&(data->acquisition.mutex));
	data->acquisition.thread = NULL;
	InitializeConditionVariable(&(data->acquisition.acquisitionFinishCondition));
	data->acquisition.running = false;
	data->acquisition.armed = false;
	data->acquisition.started = false;
	data->acquisition.stopRequested = false;
	data->acquisition.acquisition = NULL;
}


static OScDev_Error SimulateImage(OScDev_Device* device, OScDev_Acquisition* acq)
{
	uint32_t xOffset, yOffset, width, height;
	OScDev_Acquisition_GetROI(acq, &xOffset, &yOffset, &width, &height);

	bool shouldContinue;
	srand((unsigned)time(NULL));
	for (int i = 0; i < width * height; ++i)
	{
		buf_frame[i] = rand() % 256;
	}
	shouldContinue = OScDev_Acquisition_CallFrameCallback(acq, 0, buf_frame);
	Sleep(100);

	return OScDev_OK;
}


static DWORD WINAPI AcquisitionLoop(void* param)
{
	OScDev_Device* device = (OScDev_Device*)param;
	OScDev_Acquisition* acq = GetData(device)->acquisition.acquisition;

	uint32_t totalFrames = OScDev_Acquisition_GetNumberOfFrames(acq);

	uint32_t xOffset, yOffset, width, height;
	OScDev_Acquisition_GetROI(acq, &xOffset, &yOffset, &width, &height);

	buf_frame = (uint16_t*)malloc(width * height * sizeof(uint16_t));
	for (uint32_t frame = 0; frame < totalFrames; ++frame)
	{
		bool stopRequested;
		EnterCriticalSection(&(GetData(device)->acquisition.mutex));
		stopRequested = GetData(device)->acquisition.stopRequested;
		LeaveCriticalSection(&(GetData(device)->acquisition.mutex));
		if (stopRequested)
			break;

		char msg[OScDev_MAX_STR_LEN + 1];
		snprintf(msg, OScDev_MAX_STR_LEN, "Sequence acquiring frame # %d", frame);
		OScDev_Log_Debug(device, msg);

		OScDev_Error errCode;
		if (OScDev_CHECK(errCode, SimulateImage(device, acq)))
			break;
	}
	free(buf_frame);

	EnterCriticalSection(&(GetData(device)->acquisition.mutex));
	GetData(device)->acquisition.running = false;
	LeaveCriticalSection(&(GetData(device)->acquisition.mutex));
	CONDITION_VARIABLE* cv = &(GetData(device)->acquisition.acquisitionFinishCondition);
	WakeAllConditionVariable(cv);

	return 0;
}


OScDev_Error RunAcquisitionLoop(OScDev_Device* device)
{
	DWORD id;
	GetData(device)->acquisition.thread =
		CreateThread(NULL, 0, AcquisitionLoop, device, 0, &id);
	return OScDev_OK;
}


OScDev_Error WaitForAcquisitionToFinish(OScDev_Device* device)
{
	CRITICAL_SECTION* mutex = &GetData(device)->acquisition.mutex;
	CONDITION_VARIABLE* cv = &(GetData(device)->acquisition.acquisitionFinishCondition);

	EnterCriticalSection(mutex);
	while (GetData(device)->acquisition.running)
	{
		SleepConditionVariableCS(cv, mutex, INFINITE);
	}
	LeaveCriticalSection(mutex);

	return OScDev_OK;
}


OScDev_Error IsAcquisitionRunning(OScDev_Device* device, bool* isRunning)
{
	EnterCriticalSection(&(GetData(device)->acquisition.mutex));
	*isRunning = GetData(device)->acquisition.running;
	LeaveCriticalSection(&(GetData(device)->acquisition.mutex));
	return OScDev_OK;
}


OScDev_Error StopAcquisitionAndWait(OScDev_Device* device)
{
	CRITICAL_SECTION* mutex = &GetData(device)->acquisition.mutex;
	CONDITION_VARIABLE* cv = &(GetData(device)->acquisition.acquisitionFinishCondition);

	EnterCriticalSection(mutex);
	if (GetData(device)->acquisition.started) {
		GetData(device)->acquisition.stopRequested = true;
	}
	else { // Armed but not started
		GetData(device)->acquisition.running = false;
	}

	while (GetData(device)->acquisition.running)
	{
		SleepConditionVariableCS(cv, mutex, INFINITE);
	}
	LeaveCriticalSection(mutex);

	return OScDev_OK;
}


static OScDev_Error GetModelName(const char** name)
{
	*name = "SimulatedDevice_legacy_errors";
	return OScDev_OK;
}


static OScDev_Error EnumerateInstances(OScDev_PtrArray** devices)
{
	*devices = OScDev_PtrArray_Create();

	struct DevicePrivateData* data = calloc(1, sizeof(struct DevicePrivateData));

	OScDev_Error errCode;
	OScDev_Device* device0 = NULL;
	errCode = OScDev_Device_Create(&device0, &g_SimulatedDeviceImpl, data);
	if (errCode) {
		OScDev_PtrArray_Destroy(*devices);
		*devices = NULL;
		free(data);
		return errCode;
	}
	InitializeDevicePrivateData(GetData(device0));
	OScDev_PtrArray_Append(*devices, device0);

	return OScDev_OK;
}


static OScDev_Error ReleaseInstance(OScDev_Device* device)
{
	free(GetData(device));
	return OScDev_OK;
}


static OScDev_Error GetName(OScDev_Device* device, char* name)
{
	strncpy(name, "SimulatedDevice_legacy_errors", OScDev_MAX_STR_LEN);
	return OScDev_OK;
}


static OScDev_Error Open(OScDev_Device* device)
{
	return OScDev_OK;
}


static OScDev_Error Close(OScDev_Device* device)
{
	return StopAcquisitionAndWait(device);
}


static OScDev_Error HasClock(OScDev_Device* device, bool* hasClock)
{
	*hasClock = true;
	return OScDev_OK;
}


static OScDev_Error HasScanner(OScDev_Device* device, bool* hasScanner)
{
	*hasScanner = true;
	return OScDev_OK;
}


static OScDev_Error HasDetector(OScDev_Device* device, bool* hasDetector)
{
	*hasDetector = true;
	return OScDev_OK;
}


static OScDev_Error GetPixelRates(OScDev_Device* device, OScDev_NumRange** pixelRatesHz)
{
	*pixelRatesHz = OScDev_NumRange_CreateDiscrete();
	OScDev_NumRange_AppendDiscrete(*pixelRatesHz, 1e6 * 1.2500);
	return OScDev_OK;
}


static OScDev_Error GetResolutions(OScDev_Device* device, OScDev_NumRange** resolutions)
{
	*resolutions = OScDev_NumRange_CreateDiscrete();
	OScDev_NumRange_AppendDiscrete(*resolutions, 256);
	OScDev_NumRange_AppendDiscrete(*resolutions, 512);
	return OScDev_OK;
}


static OScDev_Error GetNumberOfChannels(OScDev_Device* device, uint32_t* nChannels)
{
	*nChannels = 1;
	return OScDev_OK;
}


static OScDev_Error GetBytesPerSample(OScDev_Device* device, uint32_t* bytesPerSample)
{
	*bytesPerSample = 2;
	return OScDev_OK;
}


static OScDev_Error MakeSettings(OScDev_Device* device, OScDev_PtrArray** settings)
{
	OScDev_Error err = OScDev_OK;
	*settings = OScDev_PtrArray_Create();

	OScDev_Setting* lineDelay;
	if (OScDev_CHECK(err, OScDev_Setting_Create(&lineDelay, "Error on start", OScDev_ValueType_Bool,
		&SettingImpl_ErrorOnStart, device)))
		goto error;
	OScDev_PtrArray_Append(*settings, lineDelay);
	return OScDev_OK;

error:
	for (size_t i = 0; i < OScDev_PtrArray_Size(*settings); ++i) {
		OScDev_Setting_Destroy(OScDev_PtrArray_At(*settings, i));
	}
	OScDev_PtrArray_Destroy(*settings);
	*settings = NULL;
	return err;
}


static OScDev_Error Arm(OScDev_Device* device, OScDev_Acquisition* acq)
{
	EnterCriticalSection(&(GetData(device)->acquisition.mutex));
	{
		GetData(device)->acquisition.acquisition = acq;
		GetData(device)->acquisition.stopRequested = false;
		GetData(device)->acquisition.running = true;
		GetData(device)->acquisition.armed = true;
		GetData(device)->acquisition.started = false;
	}
	LeaveCriticalSection(&(GetData(device)->acquisition.mutex));
	return OScDev_OK;
}


static OScDev_Error Start(OScDev_Device* device)
{
	EnterCriticalSection(&(GetData(device)->acquisition.mutex));
	{
		if (!GetData(device)->acquisition.running ||
			!GetData(device)->acquisition.armed)
		{
			LeaveCriticalSection(&(GetData(device)->acquisition.mutex));
			return OScDev_Error_Not_Armed;
		}

		if (GetData(device)->acquisition.started)
		{
			LeaveCriticalSection(&(GetData(device)->acquisition.mutex));
			return OScDev_Error_Acquisition_Running;
		}

		if (GetData(device)->simulatedErrorOnStart) {
			GetData(device)->acquisition.running = false;
			LeaveCriticalSection(&(GetData(device)->acquisition.mutex));
			return OScDev_Error_Unknown;
		}
		GetData(device)->acquisition.started = true;
	}
	LeaveCriticalSection(&(GetData(device)->acquisition.mutex));
	return RunAcquisitionLoop(device);
}


static OScDev_Error Stop(OScDev_Device* device)
{
	return StopAcquisitionAndWait(device);
}


static OScDev_Error IsRunning(OScDev_Device* device, bool* isRunning)
{
	return IsAcquisitionRunning(device, isRunning);
}


static OScDev_Error Wait(OScDev_Device* device)
{
	return WaitForAcquisitionToFinish(device);
}


static OScDev_DeviceImpl g_SimulatedDeviceImpl = {
	.GetModelName = GetModelName,
	.EnumerateInstances = EnumerateInstances,
	.ReleaseInstance = ReleaseInstance,
	.GetName = GetName,
	.Open = Open,
	.Close = Close,
	.HasClock = HasClock,
	.HasScanner = HasScanner,
	.HasDetector = HasDetector,
	.GetPixelRates = GetPixelRates,
	.GetResolutions = GetResolutions,
	.GetNumberOfChannels = GetNumberOfChannels,
	.GetBytesPerSample = GetBytesPerSample,
	.MakeSettings = MakeSettings,
	.Arm = Arm,
	.Start = Start,
	.Stop = Stop,
	.IsRunning = IsRunning,
	.Wait = Wait,
};


OScDev_Error GetDeviceImpls(OScDev_PtrArray** deviceImpls)
{
	*deviceImpls = OScDev_PtrArray_CreateFromNullTerminated(
		(OScDev_DeviceImpl * []) {
		&g_SimulatedDeviceImpl, NULL
	});
	return OScDev_OK;
}


OScDev_MODULE_IMPL =
{
	.displayName = "Simulated Device (legacy errors)",
	.GetDeviceImpls = GetDeviceImpls,
	.supportsRichErrors = false,
};
