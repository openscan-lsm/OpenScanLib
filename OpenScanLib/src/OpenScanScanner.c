#include "OpenScanLibPrivate.h"


OSc_Error OSc_Scanner_GetDevice(OSc_Scanner *scanner, OSc_Device **device)
{
	*device = scanner->device;
	return OSc_Error_OK;
}