#include "OpenScanLibPrivate.h"

#include <stdio.h>

static OSc_LogFunc g_logFunc;
static void *g_logData;


void OSc_LogFunc_Set(OSc_LogFunc func, void *data)
{
	g_logFunc = func;
	g_logData = data;
}


void OSc_Log(OSc_Device *device, OSc_LogLevel level, const char *message)
{
	if (OScInternal_Device_Log(device, level, message))
		return;

	if (g_logFunc) {
		g_logFunc(message, level, g_logData);
	}
	else {
		const char* lev;
		switch (level) {
		case OSc_LogLevel_Debug:
			lev = "DBG";
			break;
		case OSc_LogLevel_Info:
			lev = "INF";
			break;
		case OSc_LogLevel_Warning:
			lev = "WRN";
			break;
		case OSc_LogLevel_Error:
			lev = "ERR";
			break;
		default:
			lev = "???";
			break;
		}
		fprintf(stderr, "OpenScanLib[%s]: %s\n", lev, message);
	}
}