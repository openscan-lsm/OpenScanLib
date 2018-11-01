#include "OpenScanLibPrivate.h"

#include <stdio.h>

static OSc_Log_Func g_logFunc;
static void *g_logData;


void OSc_Set_Log_Func(OSc_Log_Func func, void *data)
{
	g_logFunc = func;
	g_logData = data;
}


void OSc_Log_Set_Device_Log_Func(OSc_Device *device, OSc_Log_Func func, void *data)
{
	device->logFunc = func;
	device->logData = data;
}


void OSc_Log(OSc_Device *device, OSc_Log_Level level, const char *message)
{
	bool deviceSpecific = device && device->logFunc;
	OSc_Log_Func func = deviceSpecific ? device->logFunc : g_logFunc;
	void *data = deviceSpecific ? device->logData : g_logData;
	if (func)
		func(message, level, data);
	else
		fprintf(stderr, "OpenScan C API: %s\n", message);
}