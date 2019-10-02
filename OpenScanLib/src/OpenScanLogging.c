#include "OpenScanLibPrivate.h"
#include "DeviceInterface.h"

#include <stdio.h>

static OSc_LogFunc g_logFunc;
static void *g_logData;


void OSc_LogFunc_Set(OSc_LogFunc func, void *data)
{
	g_logFunc = func;
	g_logData = data;
}


void OSc_Device_SetLogFunc(OSc_Device *device, OSc_LogFunc func, void *data)
{
	if (!device)
		return;

	device->logFunc = func;
	device->logData = data;
}


void OSc_Log(OSc_Device *device, OSc_LogLevel level, const char *message)
{
	bool deviceSpecific = device && device->logFunc;
	OSc_LogFunc func = deviceSpecific ? device->logFunc : g_logFunc;
	void *data = deviceSpecific ? device->logData : g_logData;
	if (func)
		func(message, level, data);
	else
		fprintf(stderr, "OpenScan C API: %s\n", message);
}