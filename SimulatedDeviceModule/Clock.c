#include "OScNIDAQDevicePrivate.h"
#include "Waveform.h"

#include <OpenScanDeviceLib.h>
#include <NIDAQmx.h>


static int32 CreateClockTasks(OScDev_Device *device, struct ClockConfig *config, OScDev_Acquisition *acq);
static int32 ConfigureClockTiming(OScDev_Device *device, struct ClockConfig *config, OScDev_Acquisition *acq);
static int32 ConfigureClockTriggers(OScDev_Device *device, struct ClockConfig *config);
static int32 WriteClockOutput(OScDev_Device *device, struct ClockConfig *config, OScDev_Acquisition *acq);


// Initialize, configure, and arm the clock, whatever its current state
int32 SetUpClock(OScDev_Device *device, struct ClockConfig *config, OScDev_Acquisition *acq)
{
	bool mustCommit = false;

	int32 nierr;
	if (!config->doTask || !config->lineCtrTask)
	{
		// In case one of the two tasks exists
		nierr = ShutdownClock(device, config);
		if (nierr)
			return nierr;

		nierr = CreateClockTasks(device, config, acq);
		if (nierr)
			return nierr;
		config->mustReconfigureTiming = true;
		config->mustReconfigureTriggers = true;
		config->mustRewriteOutput = true;
	}

	if (config->mustReconfigureTiming)
	{
		nierr = ConfigureClockTiming(device, config, acq);
		if (nierr)
			goto error;
		config->mustReconfigureTiming = false;
		mustCommit = true;
	}

	if (config->mustReconfigureTriggers)
	{
		nierr = ConfigureClockTriggers(device, config);
		if (nierr)
			goto error;
		config->mustReconfigureTriggers = false;
		mustCommit = true;
	}

	if (config->mustRewriteOutput)
	{
		nierr = WriteClockOutput(device, config, acq);
		if (nierr)
			goto error;
		config->mustRewriteOutput = false;
		mustCommit = true;
	}

	if (mustCommit)
	{
		nierr = DAQmxTaskControl(config->doTask, DAQmx_Val_Task_Commit);
		if (nierr)
		{
			LogNiError(device, nierr, "committing clock do task");
			goto error;
		}

		nierr = DAQmxTaskControl(config->lineCtrTask, DAQmx_Val_Task_Commit);
		if (nierr)
		{
			LogNiError(device, nierr, "committing clock lineCtr task");
			goto error;
		}
	}

	return 0;

error:
	if (ShutdownClock(device, config))
		OScDev_Log_Error(device, "Failed to clean up clock task(s) after error");
	return nierr;
}


// Remove all DAQmx configuration for the clock
int32 ShutdownClock(OScDev_Device *device, struct ClockConfig *config)
{
	int32 nierr1 = 0, nierr2 = 0;

	if (config->doTask)
	{
		nierr1 = DAQmxClearTask(config->doTask);
		if (nierr1)
			LogNiError(device, nierr1, "clearing clock do task");
		config->doTask = 0;
	}

	if (config->lineCtrTask)
	{
		nierr2 = DAQmxClearTask(config->lineCtrTask);
		if (nierr2)
			LogNiError(device, nierr2, "clearing clock lineCtr task");
		config->lineCtrTask = 0;
	}

	if (nierr1)
		return nierr1;
	return nierr2;
}


int32 StartClock(OScDev_Device *device, struct ClockConfig *config)
{
	int32 nierr;

	nierr = DAQmxStartTask(config->doTask);
	if (nierr)
	{
		LogNiError(device, nierr, "starting clock do task");
		ShutdownClock(device, config);
		return nierr;
	}

	nierr = DAQmxStartTask(config->lineCtrTask);
	if (nierr)
	{
		LogNiError(device, nierr, "starting clock lineCtr task");
		ShutdownClock(device, config);
		return nierr;
	}

	return 0;
}


int32 StopClock(OScDev_Device *device, struct ClockConfig *config)
{
	int32 nierr;

	nierr = DAQmxStopTask(config->doTask);
	if (nierr)
	{
		LogNiError(device, nierr, "stopping clock do task");
		ShutdownClock(device, config);
		return nierr;
	}

	nierr = DAQmxStopTask(config->lineCtrTask);
	if (nierr)
	{
		LogNiError(device, nierr, "stopping clock lineCtr task");
		ShutdownClock(device, config);
		return nierr;
	}

	return 0;
}


static int32 CreateClockTasks(OScDev_Device *device, struct ClockConfig *config, OScDev_Acquisition *acq)
{
	int32 nierr;
	nierr = DAQmxCreateTask("ClockDO", &config->doTask);
	if (nierr)
	{
		LogNiError(device, nierr, "creating clock do task");
		return nierr;
	}

	// P0.5 = line clock
	// P0.6 = inverted line clock (for FLIM)
	// P0.7 = frame clock
	// This needs to be port0 to support buffered output

	char doTerminals[256];
	strncpy(doTerminals, GetData(device)->deviceName, sizeof(doTerminals) - 1);
	strncat(doTerminals, "/port0/line5:7",
		sizeof(doTerminals) - strlen(doTerminals) - 1);
	nierr = DAQmxCreateDOChan(config->doTask, doTerminals, "ClockDO",
		DAQmx_Val_ChanPerLine);
	if (nierr)
	{
		LogNiError(device, nierr, "creating clock do channel");
		return nierr;
	}

	nierr = DAQmxGetReadNumChans(config->doTask, &GetData(device)->numDOChannels);
	if (nierr)
	{
		LogNiError(device, nierr, "getting number of channels from clock do task");
		return nierr;
	}

	nierr = DAQmxCreateTask("ClockCtr", &config->lineCtrTask);
	if (nierr)
	{
		LogNiError(device, nierr, "creating clock lineCtr task");
		return nierr;
	}

	double pixelRateHz = OScDev_Acquisition_GetPixelRate(acq);
	uint32_t xOffset, yOffset, width, height;
	OScDev_Acquisition_GetROI(acq, &xOffset, &yOffset, &width, &height);

	uint32_t elementsPerLine = GetData(device)->lineDelay + width + X_RETRACE_LEN;
	double effectiveScanPortion = (double)width / elementsPerLine;
	double lineFreqHz = pixelRateHz / GetData(device)->binFactor / elementsPerLine;
	double scanPhase = GetData(device)->binFactor / pixelRateHz * GetData(device)->lineDelay;

	char ctrTerminals[256];
	strncpy(ctrTerminals, GetData(device)->deviceName, sizeof(ctrTerminals) - 1);
	strncat(ctrTerminals, "/ctr0",
		sizeof(ctrTerminals) - strlen(ctrTerminals) - 1);
	nierr = DAQmxCreateCOPulseChanFreq(config->lineCtrTask, ctrTerminals, "ClockLineCTR",
		DAQmx_Val_Hz, DAQmx_Val_Low, scanPhase, lineFreqHz, effectiveScanPortion);
	if (nierr)
	{
		LogNiError(device, nierr, "creating clock co pulse channel");
		return nierr;
	}

	return 0;
}


static int32 ConfigureClockTiming(OScDev_Device *device, struct ClockConfig *config, OScDev_Acquisition *acq)
{
	int32 nierr;

	double pixelRateHz = OScDev_Acquisition_GetPixelRate(acq);
	uint32_t xOffset, yOffset, width, height;
	OScDev_Acquisition_GetROI(acq, &xOffset, &yOffset, &width, &height);

	uint32_t elementsPerLine = GetData(device)->lineDelay + width + X_RETRACE_LEN;
	uint32_t yLen = height + Y_RETRACE_LEN;
	int32 elementsPerFramePerChan = elementsPerLine * height;
	int32 totalElementsPerFramePerChan = elementsPerLine * yLen;

	nierr = DAQmxCfgSampClkTiming(config->doTask, "",
		pixelRateHz / GetData(device)->binFactor,
		DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, elementsPerFramePerChan);
	if (nierr)
	{
		LogNiError(device, nierr, "configuring timing for clock do task");
		return nierr;
	}

	double effectiveScanPortion = (double)width / elementsPerLine;
	double lineFreqHz = pixelRateHz / GetData(device)->binFactor / elementsPerLine;
	double scanPhase = GetData(device)->binFactor / pixelRateHz * GetData(device)->lineDelay;

	nierr = DAQmxSetChanAttribute(config->lineCtrTask, "",
		DAQmx_CO_Pulse_Freq, lineFreqHz);
	if (nierr)
	{
		LogNiError(device, nierr, "setting clock lineCtr frequency");
		return nierr;
	}

	nierr = DAQmxSetChanAttribute(config->lineCtrTask, "",
		DAQmx_CO_Pulse_Freq_InitialDelay, scanPhase);
	if (nierr)
	{
		LogNiError(device, nierr, "setting clock lineCtr initial delay");
		return nierr;
	}

	nierr = DAQmxSetChanAttribute(config->lineCtrTask, "",
		DAQmx_CO_Pulse_DutyCyc, effectiveScanPortion);
	if (nierr)
	{
		LogNiError(device, nierr, "setting clock lineCtr duty cycle");
		return nierr;
	}

	nierr = DAQmxCfgImplicitTiming(config->lineCtrTask, DAQmx_Val_FiniteSamps,
		height);
	if (nierr)
	{
		LogNiError(device, nierr, "configuring timing for clock lineCtr");
		return nierr;
	}

	return 0;
}


static int32 ConfigureClockTriggers(OScDev_Device *device, struct ClockConfig *config)
{
	char triggerSource[256] = "/";
	strncat(triggerSource, GetData(device)->deviceName,
		sizeof(triggerSource) - strlen(triggerSource) - 1);
	strncat(triggerSource, "/ao/StartTrigger",
		sizeof(triggerSource) - strlen(triggerSource) - 1);

	int32 nierr;
	nierr = DAQmxCfgDigEdgeStartTrig(config->doTask,
		triggerSource, DAQmx_Val_Rising);
	if (nierr)
	{
		LogNiError(device, nierr, "configuring trigger for clock do task");
		return nierr;
	}

	nierr = DAQmxSetStartTrigRetriggerable(config->doTask, 1);
	if (nierr)
	{
		LogNiError(device, nierr, "setting retriggerable clock do task");
		return nierr;
	}

	nierr = DAQmxCfgDigEdgeStartTrig(config->lineCtrTask,
		triggerSource, DAQmx_Val_Rising);
	if (nierr)
	{
		LogNiError(device, nierr, "configuring trigger for clock lineCtr task");
		return nierr;
	}

	return 0;
}


static int32 WriteClockOutput(OScDev_Device *device, struct ClockConfig *config, OScDev_Acquisition *acq)
{
	uint32_t xOffset, yOffset, width, height;
	OScDev_Acquisition_GetROI(acq, &xOffset, &yOffset, &width, &height);

	uint32_t elementsPerLine = GetData(device)->lineDelay + width + X_RETRACE_LEN;
	uint32_t yLen = height + Y_RETRACE_LEN;
	int32 elementsPerFramePerChan = elementsPerLine * height;  // without y retrace portion
	int32 totalElementsPerFramePerChan = elementsPerLine * yLen;   // including y retrace portion

	// Q: Why do we use elementsPerFramePerChan, not totalElementsPerFramePerChan?

	// digital line clock pattern for triggering acquisition line by line
	uInt8 *lineClockPattern = (uInt8*)malloc(elementsPerFramePerChan);
	// digital line clock pattern for FLIM
	uInt8 *lineClockFLIM = (uInt8*)malloc(elementsPerFramePerChan);
	// digital frame clock pattern for FLIM
	uInt8 *frameClockFLIM = (uInt8*)malloc(elementsPerFramePerChan);
	// combination of lineClock, lineClockFLIM, and frameClock
	uInt8 *lineClockPatterns = (uInt8*)malloc(
		elementsPerFramePerChan * GetData(device)->numDOChannels);

	// TODO: why use elementsPerLine instead of elementsPerFramePerChan?
	int err = GenerateLineClock(width, height,
		GetData(device)->lineDelay,	lineClockPattern);
	if (err != 0)
		return OScDev_Error_Waveform_Out_Of_Range;
	err = GenerateFLIMLineClock(width, height,
		GetData(device)->lineDelay, lineClockFLIM);
	if (err != 0)
		return OScDev_Error_Waveform_Out_Of_Range;
	err = GenerateFLIMFrameClock(width, height,
		GetData(device)->lineDelay, frameClockFLIM);
	if (err != 0)
		return OScDev_Error_Waveform_Out_Of_Range;

	// combine line, inverted line, and frame clocks
	// TODO: make it more generic
	for (int i = 0; i < elementsPerFramePerChan; i++)
	{
		lineClockPatterns[i] = lineClockPattern[i];
		lineClockPatterns[i + elementsPerFramePerChan] = lineClockFLIM[i];
		lineClockPatterns[i + 2 * elementsPerFramePerChan] = frameClockFLIM[i];
	}

	int32 numWritten = 0;
	int32 nierr = DAQmxWriteDigitalLines(config->doTask,
		elementsPerFramePerChan, FALSE, 10.0,
		DAQmx_Val_GroupByChannel, lineClockPatterns, &numWritten, NULL);
	if (nierr)
	{
		LogNiError(device, nierr, "writing clock do waveforms");
		goto cleanup;
	}
	if (numWritten != elementsPerFramePerChan)
	{
		OScDev_Log_Error(device, "Failed to write complete clock waveform");
		goto cleanup;
	}

cleanup:
	free(lineClockPattern);
	free(lineClockFLIM);
	free(frameClockFLIM);
	free(lineClockPatterns);
	return nierr;
}