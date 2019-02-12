#include "OpenScanLibPrivate.h"


OSc_Error OSc_Clock_Get_Device(OSc_Clock *clock, OSc_Device **device)
{
	*device = clock->device;
	return OSc_Error_OK;
}