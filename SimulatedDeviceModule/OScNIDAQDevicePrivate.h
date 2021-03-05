#pragma once
#include "strmap.h"

#include "OpenScanDeviceLib.h"

#include <NIDAQmx.h>

#include <Windows.h>

#define OSc_DEFAULT_RESOLUTION 512
#define OSc_DEFAULT_ZOOM 1.0
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
	double configuredPixelRateHz;
	uint32_t configuredResolution;
	double configuredZoomFactor;
	uint32_t configuredXOffset, configuredYOffset;
	uint32_t configuredRasterWidth, configuredRasterHeight;

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
	StrMap* channelMap_;

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
	float64 *rawDataBuffer;
	size_t rawDataSize; // Current data size
	size_t rawDataCapacity; // Buffer size

	// Per-channel frame buffers that we fill in and pass to OpenScanLib
	// Index is order among currently enabled channels.
	// Buffers for unused channels may not be allocated.
	uint16_t *frameBuffers[OSc_Total_Channel_Num];
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
		OScDev_Acquisition *acquisition;
	} acquisition;
};


static inline struct OScNIDAQPrivateData *GetData(OScDev_Device *device)
{
	return (struct OScNIDAQPrivateData *)OScDev_Device_GetImplData(device);
}


OScDev_Error EnumerateInstances(OScDev_PtrArray **devices, OScDev_DeviceImpl *impl);
OScDev_Error NIDAQMakeSettings(OScDev_Device *device, OScDev_PtrArray **settings);
OScDev_Error GetSelectedDispChannels(OScDev_Device *device);


int32 SetUpClock(OScDev_Device *device, struct ClockConfig *config, OScDev_Acquisition *acq);
int32 ShutdownClock(OScDev_Device *device, struct ClockConfig *config);
int32 StartClock(OScDev_Device *device, struct ClockConfig *config);
int32 StopClock(OScDev_Device *device, struct ClockConfig *config);
int32 SetUpScanner(OScDev_Device *device, struct ScannerConfig *config, OScDev_Acquisition *acq);
int32 ShutdownScanner(OScDev_Device *device, struct ScannerConfig *config);
int32 StartScanner(OScDev_Device *device, struct ScannerConfig *config);
int32 StopScanner(OScDev_Device *device, struct ScannerConfig *config);
int32 SetUpDetector(OScDev_Device *device, struct DetectorConfig *config, OScDev_Acquisition *acq);
int32 ShutdownDetector(OScDev_Device *device, struct DetectorConfig *config);
int32 StartDetector(OScDev_Device *device, struct DetectorConfig *config);
int32 StopDetector(OScDev_Device *device, struct DetectorConfig *config);



// Must be called immediately after failed DAQmx function
void LogNiError(OScDev_Device *device, int32 nierr, const char *when);