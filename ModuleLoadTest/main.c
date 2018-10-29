#include "OpenScanLib.h"

#include <stdio.h>


int main()
{
	char *paths[2];
	paths[0] = ".";
	paths[1] = NULL;
	OSc_DeviceModule_Set_Search_Paths(paths);

	size_t count;
	OSc_Error err;
	if (OSc_Check_Error(err, OSc_Devices_Get_Count(&count)))
	{
		fprintf(stderr, "Could not get count of devices");
		return 1;
	}

	printf("Count of devices = %zu", count);

	OSc_Device **devices;
	if (OSc_Check_Error(err, OSc_Devices_Get_All(&devices, &count)))
	{
		fprintf(stderr, "Could not get all devices");
		return 1;
	}

	return 0;
}