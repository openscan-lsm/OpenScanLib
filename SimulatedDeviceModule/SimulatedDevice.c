#include "OpenScanDeviceLib.h"

#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <stdio.h>
#include <time.h>
#include <NIDAQmx.h>

static const uint32_t X_RETRACE_LEN = 128;

#define OSc_Total_Channel_Num 3

// DAQmx tasks and flags to track invalidated configurations for clock
// See Clock.c
struct ClockConfig
{
	TaskHandle doTask;
	TaskHandle lineCtrTask;
	bool mustReconfigureTiming;
	bool mustReconfigureTriggers;
	bool mustRewriteOutput;
};


// DAQmx task and flags to track invalidated configurations for scanner
// See Scanner.c
struct ScannerConfig
{
	TaskHandle aoTask;
	bool mustReconfigureTiming;
	bool mustRewriteOutput;
};


// DAQmx task and flags to track invalidated configurations for detector
// See Detector.c
struct DetectorConfig
{
	TaskHandle aiTask;
	bool mustReconfigureTiming;
	bool mustReconfigureTrigger;
	bool mustReconfigureCallback;
};


struct OScNIDAQPrivateData
{
	// The DAQmx name for the DAQ card
	char deviceName[OScDev_MAX_STR_LEN + 1];

	struct ClockConfig clockConfig;
	struct ScannerConfig scannerConfig;
	struct DetectorConfig detectorConfig;

	bool oneFrameScanDone;
	// Flags for scanner and detector
	bool detectorOnly;
	bool scannerOnly;

	// counted as number of pixels. 
	// to adjust for the lag between the mirror control signal and the actual position of the mirror
	// scan phase (uSec) = line delay * bin factor / scan rate
	uint32_t lineDelay;
	uint32_t binFactor;
	uint32_t numLinesToBuffer;
	double inputVoltageRange;
	uInt32 numAIChannels;
	uInt32 numDOChannels; // reserved for multiple line and frame clocks
	double offsetXY[2];
	double minVolts_; // min possible for device
	double maxVolts_; // max possible for device
	uint32_t channelCount;

	char** aiPorts_;
	char* aoChanList_;
	char* doChanList_;
	char* coChanList_;
	char* acqTrigPort_;
	const char** selectedDispChan_;
	char* enabledAIPorts_;
	//StrMap* channelMap_;

	enum {
		CHANNEL1,
		CHANNEL2,
		CHANNEL3,
		CHANNELS_1_AND_2,
		CHANNELS_1_AND_3,
		CHANNELS1_2_3,
		CHANNELS_NUM_VALUES
	} channels;

	// Read, but unprocessed, raw samples; channels interleaved
	// Leftover data from the previous read, if any, is at the start of the
	// buffer and consists of rawDataSize samples.
	float64* rawDataBuffer;
	size_t rawDataSize; // Current data size
	size_t rawDataCapacity; // Buffer size

	// Per-channel frame buffers that we fill in and pass to OpenScanLib
	// Index is order among currently enabled channels.
	// Buffers for unused channels may not be allocated.
	uint16_t* frameBuffers[OSc_Total_Channel_Num];
	size_t framePixelsFilled;

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


static inline struct OScNIDAQPrivateData* GetData(OScDev_Device* device)
{
	return (struct OScNIDAQPrivateData*)OScDev_Device_GetImplData(device);
}


static void PopulateDefaultParameters(struct OScNIDAQPrivateData *data)
{
	data->detectorOnly = false;
	data->scannerOnly = false;
	//data->channelMap_ = sm_new(32);
	//Assume portList[256][32];
	data->aiPorts_ = malloc(256 * (sizeof(char)*32));
	for (int i = 0; i < 256; i++) {
		data->aiPorts_[i] = malloc(32 * sizeof(char));
	}
	data->enabledAIPorts_ = malloc(sizeof(char) * 2048);
	
	// Assume 3 chanel at maximum, each 16 chars at most
	data->selectedDispChan_ = malloc(3 * sizeof(char*));
	for (int i = 0; i < 3; i++) {
		data->selectedDispChan_[i] = malloc(sizeof(char) * 16);
	}
	data->selectedDispChan_[0] = "Channel1";
	data->channelCount = 1;

	memset(&data->clockConfig, 0, sizeof(struct ClockConfig));
	memset(&data->scannerConfig, 0, sizeof(struct ScannerConfig));
	memset(&data->detectorConfig, 0, sizeof(struct DetectorConfig));

	data->oneFrameScanDone = false;
	data->framePixelsFilled = 0;

	data->lineDelay = 50;
	data->binFactor = 2;
	data->numLinesToBuffer = 8;
	data->inputVoltageRange = 10.0;
	data->channels = CHANNEL1;
	data->numAIChannels = 1;
	data->numDOChannels = 1;
	data->offsetXY[0] = 0.0;
	data->offsetXY[1] = 0.0;
	data->minVolts_ = -10.0;
	data->maxVolts_ = 10.0;
	
	InitializeCriticalSection(&(data->acquisition.mutex));
	data->acquisition.thread = NULL;
	InitializeConditionVariable(&(data->acquisition.acquisitionFinishCondition));
	data->acquisition.running = false;
	data->acquisition.armed = false;
	data->acquisition.started = false;
	data->acquisition.stopRequested = false;
	data->acquisition.acquisition = NULL;
}


static OScDev_RichError *SimulateImage(OScDev_Device* device, OScDev_Acquisition* acq)
{
	uint32_t xOffset, yOffset, width, height;
	OScDev_Acquisition_GetROI(acq, &xOffset, &yOffset, &width, &height);

	uint32_t elementsPerLine = GetData(device)->lineDelay + width + X_RETRACE_LEN;
	uint32_t scanLines = height;
	int32 elementsPerFramePerChan = elementsPerLine * scanLines;
	size_t nPixels = width * height;

	GetData(device)->oneFrameScanDone = false;
	GetData(device)->framePixelsFilled = 0;

	bool shouldContinue;
	srand((unsigned)time(NULL));
	uint16_t* buf = (uint16_t*)malloc(width * height * sizeof(uint16_t));
	for (int i = 0; i < width * height; ++i)
	{
		buf[i] = rand() % 256;
	}
	for (uint32_t ch = 0; ch < GetData(device)->numAIChannels; ++ch)
	{
		shouldContinue = OScDev_Acquisition_CallFrameCallback(acq, ch, buf);
	}
	Sleep(100);

	return OScDev_RichError_OK;
}


static DWORD WINAPI AcquisitionLoop(void *param)
{
	OScDev_Device *device = (OScDev_Device *)param;
	OScDev_Acquisition *acq = GetData(device)->acquisition.acquisition;

	uint32_t totalFrames = OScDev_Acquisition_GetNumberOfFrames(acq);

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

		OScDev_RichError *err;
		if (OScDev_CHECK(err, SimulateImage(device, acq)))
		{
			char msg[OScDev_MAX_STR_LEN + 1];
			snprintf(msg, OScDev_MAX_STR_LEN, "Error during sequence acquisition: %d", (int)err);
			OScDev_Log_Error(device, msg);
			break;
		}
	}

	EnterCriticalSection(&(GetData(device)->acquisition.mutex));
	GetData(device)->acquisition.running = false;
	LeaveCriticalSection(&(GetData(device)->acquisition.mutex));
	CONDITION_VARIABLE *cv = &(GetData(device)->acquisition.acquisitionFinishCondition);
	WakeAllConditionVariable(cv);

	return 0;
}


OScDev_RichError *RunAcquisitionLoop(OScDev_Device* device)
{
	DWORD id;
	GetData(device)->acquisition.thread =
		CreateThread(NULL, 0, AcquisitionLoop, device, 0, &id);
	return OScDev_RichError_OK;
}


OScDev_RichError *WaitForAcquisitionToFinish(OScDev_Device *device)
{
	CRITICAL_SECTION *mutex = &GetData(device)->acquisition.mutex;
	CONDITION_VARIABLE *cv = &(GetData(device)->acquisition.acquisitionFinishCondition);

	EnterCriticalSection(mutex);
	while (GetData(device)->acquisition.running)
	{
		SleepConditionVariableCS(cv, mutex, INFINITE);
	}
	LeaveCriticalSection(mutex);

	return OScDev_RichError_OK;
}


OScDev_RichError *IsAcquisitionRunning(OScDev_Device *device, bool *isRunning)
{
	EnterCriticalSection(&(GetData(device)->acquisition.mutex));
	*isRunning = GetData(device)->acquisition.running;
	LeaveCriticalSection(&(GetData(device)->acquisition.mutex));
	return OScDev_RichError_OK;
}


OScDev_RichError *StopAcquisitionAndWait(OScDev_Device *device)
{
	CRITICAL_SECTION *mutex = &GetData(device)->acquisition.mutex;
	CONDITION_VARIABLE *cv = &(GetData(device)->acquisition.acquisitionFinishCondition);

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

	return OScDev_RichError_OK;
}


static OScDev_DeviceImpl g_SimulatedDeviceImpl;


static OScDev_Error GetModelName(const char **name)
{
	*name = "SimulatedDevice";
	return OScDev_OK;
}


// todo
static OScDev_Error EnumerateInstances(OScDev_PtrArray **devices)
{
	*devices = OScDev_PtrArray_Create();

	struct OScNIDAQPrivateData *data = calloc(1, sizeof(struct OScNIDAQPrivateData));
	strncpy(data->deviceName, "simulated device", OScDev_MAX_STR_LEN);

	OScDev_Error errCode;
	OScDev_Device *device0 = NULL;
	errCode = OScDev_Device_Create(&device0, &g_SimulatedDeviceImpl, data);
	if (errCode) {
		OScDev_PtrArray_Destroy(*devices);
		*devices = NULL;
		return errCode;
	}
	PopulateDefaultParameters(GetData(device0));
	OScDev_PtrArray_Append(*devices, device0);

	return OScDev_OK;
}


static OScDev_Error ReleaseInstance(OScDev_Device *device)
{
	free(GetData(device));
	return OScDev_OK;
}


static OScDev_Error GetName(OScDev_Device *device, char *name)
{
	strncpy(name, "SimulatedDevice", OScDev_MAX_STR_LEN);
	return OScDev_OK;
}

// leave?
static OScDev_Error Open(OScDev_Device* device)
{
	return OScDev_OK;
}

// leave?
static OScDev_Error Close(OScDev_Device* device)
{
	StopAcquisitionAndWait(device);
	return OScDev_OK;
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

// todo
static OScDev_Error GetPixelRates(OScDev_Device* device, OScDev_NumRange** pixelRatesHz)
{
	*pixelRatesHz = OScDev_NumRange_CreateDiscrete();
	OScDev_NumRange_AppendDiscrete(*pixelRatesHz, 1e6 * 1.2500);
	return OScDev_OK;
}

// todo
static OScDev_Error GetResolutions(OScDev_Device* device, OScDev_NumRange** resolutions)
{
	*resolutions = OScDev_NumRange_CreateDiscrete();
	OScDev_NumRange_AppendDiscrete(*resolutions, 256);
	OScDev_NumRange_AppendDiscrete(*resolutions, 512);
	return OScDev_OK;
}

// todo
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

// todo
static OScDev_Error MakeSettings(OScDev_Device* device, OScDev_PtrArray** settings)
{
	*settings = OScDev_PtrArray_Create();
	return OScDev_OK;
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

// todo
static OScDev_Error Start(OScDev_Device* device)
{
	EnterCriticalSection(&(GetData(device)->acquisition.mutex));
	{
		// bool running = GetData(device)->acquisition.running;
		// bool armed = GetData(device)->acquisition.armed;
		if (!GetData(device)->acquisition.running ||
			!GetData(device)->acquisition.armed)
		{
			LeaveCriticalSection(&(GetData(device)->acquisition.mutex));
			return OScDev_Error_ReturnAsCode(OScDev_Error_Create("not armed"));
		}
		if (GetData(device)->acquisition.started)
		{
			LeaveCriticalSection(&(GetData(device)->acquisition.mutex));
			return OScDev_Error_ReturnAsCode(OScDev_Error_Create("acquisition running"));
		}

		GetData(device)->acquisition.started = true;
	}
	LeaveCriticalSection(&(GetData(device)->acquisition.mutex));

	return OScDev_Error_ReturnAsCode(RunAcquisitionLoop(device));
}


static OScDev_Error Stop(OScDev_Device* device)
{
	// return OScDev_OK;
	return StopAcquisitionAndWait(device);
}


static OScDev_Error IsRunning(OScDev_Device* device, bool* isRunning)
{
	// *isRunning = false;
	// return OScDev_OK;
	return IsAcquisitionRunning(device, isRunning);
}


static OScDev_Error Wait(OScDev_Device* device)
{
	// return OScDev_OK;
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
	// Other required methods omitted.
};


OScDev_Error GetDeviceImpls(OScDev_PtrArray **deviceImpls)
{
	*deviceImpls = OScDev_PtrArray_CreateFromNullTerminated(
		(OScDev_DeviceImpl *[]){ &g_SimulatedDeviceImpl, NULL });
	return OScDev_OK;
}


OScDev_MODULE_IMPL =
{
	.displayName = "Simulated Device Module for OpenScan",
	.GetDeviceImpls = GetDeviceImpls,
	.supportsRichErrors = true,
};

