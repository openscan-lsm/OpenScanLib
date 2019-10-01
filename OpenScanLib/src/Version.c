#include "OpenScanLib.h"


bool OScInternal_CheckVersion(uint32_t version)
{
	uint16_t dllMajor = OScInternal_ABI_VERSION >> 16;
	uint16_t dllMinor = OScInternal_ABI_VERSION & 0xffff;
	uint16_t appMajor = version >> 16;
	uint16_t appMinor = version & 0xffff;

	if (appMajor != dllMajor)
		return false;
	if (appMajor < dllMajor)
		return false;
	return true;
}