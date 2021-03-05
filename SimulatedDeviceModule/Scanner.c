#include "OScNIDAQDevicePrivate.h"
#include "Waveform.h"

#include <OpenScanDeviceLib.h>
#include <NIDAQmx.h>
#include <stdbool.h>
#include <string.h>


static int32 ConfigureScannerTiming(OScDev_Device *device, struct ScannerConfig *config, OScDev_Acquisition *acq);
static int32 WriteScannerOutput(OScDev_Device *device, struct ScannerConfig *config, OScDev_Acquisition *acq);


// Initialize, configure, and arm the scanner, whatever its current state
int32 SetUpScanner(OScDev_Device *device, struct ScannerConfig *config, OScDev_Acquisition *acq)
{
	bool mustCommit = false;

	int32 nierr;
	if (!config->aoTask)
	{
		nierr = DAQmxCreateTask("Scanner", &config->aoTask);
		if (nierr)
		{
			LogNiError(device, nierr, "creating scanner task");
			return nierr;
		}

		char aoTerminals[256];
		strncpy(aoTerminals, GetData(device)->deviceName, sizeof(aoTerminals) - 1);
		strncat(aoTerminals, "/ao0:1", sizeof(aoTerminals) - strlen(aoTerminals) - 1);

		nierr = DAQmxCreateAOVoltageChan(config->aoTask, aoTerminals,
			"Galvos", -10.0, 10.0, DAQmx_Val_Volts, NULL);
		if (nierr)
		{
			LogNiError(device, nierr, "creating ao channels for scanner");
			goto error;
		}

		config->mustReconfigureTiming = true;
		config->mustRewriteOutput = true;
		mustCommit = true;
	}

	if (config->mustReconfigureTiming)
	{
		nierr = ConfigureScannerTiming(device, config, acq);
		if (nierr)
			goto error;
		config->mustReconfigureTiming = false;
		mustCommit = true;
	}

	if (config->mustRewriteOutput)
	{
		nierr = WriteScannerOutput(device, config, acq);
		if (nierr)
			goto error;
		config->mustRewriteOutput = false;
		mustCommit = true;
	}

	if (mustCommit)
	{
		nierr = DAQmxTaskControl(config->aoTask, DAQmx_Val_Task_Commit);
		if (nierr)
		{
			LogNiError(device, nierr, "committing task for scanner");
			goto error;
		}
	}

	return 0;

error:
	if (ShutdownScanner(device, config))
		OScDev_Log_Error(device, "Failed to clean up scanner task after error");
	return nierr;
}


// Remove all DAQmx configuration for the scanner
int32 ShutdownScanner(OScDev_Device *device, struct ScannerConfig *config)
{
	if (config->aoTask)
	{
		int32 nierr = DAQmxClearTask(config->aoTask);
		if (nierr)
		{
			LogNiError(device, nierr, "clearing scanner task");
			return nierr;
		}
		config->aoTask = 0;
	}
	return 0;
}


int32 StartScanner(OScDev_Device *device, struct ScannerConfig *config)
{
	int32 nierr;
	nierr = DAQmxStartTask(config->aoTask);
	if (nierr)
	{
		LogNiError(device, nierr, "starting scanner task");
		ShutdownScanner(device, config); // Force re-setup next time
		return nierr;
	}
	return 0;
}


int32 StopScanner(OScDev_Device *device, struct ScannerConfig *config)
{
	int32 nierr;
	nierr = DAQmxStopTask(config->aoTask);
	if (nierr)
	{
		LogNiError(device, nierr, "stopping scanner task");
		ShutdownScanner(device, config); // Force re-setup next time
		return nierr;
	}
	return 0;
}


static int32 ConfigureScannerTiming(OScDev_Device *device, struct ScannerConfig *config, OScDev_Acquisition *acq)
{
	double pixelRateHz = OScDev_Acquisition_GetPixelRate(acq);
	uint32_t xOffset, yOffset, width, height;
	OScDev_Acquisition_GetROI(acq, &xOffset, &yOffset, &width, &height);

	uint32_t elementsPerLine = GetData(device)->lineDelay + width + X_RETRACE_LEN;
	uint32_t yLen = height + Y_RETRACE_LEN;
	int32 elementsPerFramePerChan = elementsPerLine * height;
	int32 totalElementsPerFramePerChan = elementsPerLine * yLen;

	int32 nierr = DAQmxCfgSampClkTiming(config->aoTask, "",
		pixelRateHz / GetData(device)->binFactor,
		DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, totalElementsPerFramePerChan);
	if (nierr)
	{
		LogNiError(device, nierr, "configuring timing for scanner");
		return nierr;
	}

	return 0;
}


static int32 WriteScannerOutput(OScDev_Device *device, struct ScannerConfig *config, OScDev_Acquisition *acq)
{
	uint32_t resolution = OScDev_Acquisition_GetResolution(acq);
	double zoomFactor = OScDev_Acquisition_GetZoomFactor(acq);
	uint32_t xOffset, yOffset, width, height;
	OScDev_Acquisition_GetROI(acq, &xOffset, &yOffset, &width, &height);

	uint32_t elementsPerLine = GetData(device)->lineDelay + width + X_RETRACE_LEN;
	uint32_t yLen = height + Y_RETRACE_LEN;
	int32 elementsPerFramePerChan = elementsPerLine * height;  // without y retrace portion
	int32 totalElementsPerFramePerChan = elementsPerLine * yLen;   // including y retrace portion

	double *xyWaveformFrame = (double*)malloc(sizeof(double) * totalElementsPerFramePerChan * 2);

	int err = GenerateGalvoWaveformFrame(resolution, zoomFactor,
		GetData(device)->lineDelay,
		xOffset, yOffset, width, height,
		GetData(device)->offsetXY[0],
		GetData(device)->offsetXY[1],
		xyWaveformFrame);
	if (err != 0)
		return OScDev_Error_Waveform_Out_Of_Range;

	int32 numWritten = 0;
	int32 nierr = nierr = DAQmxWriteAnalogF64(config->aoTask,
		totalElementsPerFramePerChan, FALSE, 10.0,
		DAQmx_Val_GroupByChannel, xyWaveformFrame, &numWritten, NULL);
	if (nierr)
	{
		LogNiError(device, nierr, "writing scanner waveforms");
		goto cleanup;
	}
	if (numWritten != totalElementsPerFramePerChan)
	{
		OScDev_Log_Error(device, "Failed to write complete scan waveform");
		goto cleanup;
	}

cleanup:
	free(xyWaveformFrame);
	return nierr;
}