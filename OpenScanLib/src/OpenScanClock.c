#include "OpenScanLibPrivate.h"


OSc_Error OSc_Clock_GetDevice(OSc_Clock *clock, OSc_Device **device)
{
	*device = clock->device;
	return OSc_Error_OK;
}