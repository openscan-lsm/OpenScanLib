#include "OpenScanLib.h"

#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#endif

/*
 * Hint: Set the Visual Studio project property Debugging > Working Directory
 * to $(OutDir), so that TestDeviceModule.osdev can be found.
 * The Working Directory setting is stored in the .vcxproj.user file, so it
 * is not stored in Git and needs to be set in each working copy.
 */

int main()
{
	if (!OSc_Check_Version())
	{
		fprintf(stderr, "OpenScanLib ABI version mismatch\n");
		return 1;
	}

	char currentDir[512];
#ifdef _WIN32
	DWORD len = GetCurrentDirectoryA(sizeof(currentDir), currentDir);
	if (len == 0 || len > sizeof(currentDir)) {
		fprintf(stderr, "Could not get the current directory\n");
	}
#else
#error TODO
#endif
	printf("Current directory: %s\n", currentDir);

	printf("Using \".\" as search path\n");
	char *paths[2];
	paths[0] = ".";
	paths[1] = NULL;
	OSc_DeviceModule_Set_Search_Paths(paths);

	size_t count;
	OSc_Error err;
	if (OSc_Check_Error(err, OSc_Devices_Get_Count(&count)))
	{
		fprintf(stderr, "Could not get count of devices\n");
		return 1;
	}

	printf("Count of devices = %zu\n", count);

	OSc_Device **devices;
	if (OSc_Check_Error(err, OSc_Devices_Get_All(&devices, &count)))
	{
		fprintf(stderr, "Could not get all devices\n");
		return 1;
	}

	for (size_t i = 0; i < count; ++i) {
		const char *name;
		OSc_Device_Get_Name(devices[i], &name);
		printf("Device %zu: %s\n", i, name);
	}

	return 0;
}