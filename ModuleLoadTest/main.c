#include "OpenScanLib.h"

#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <RichErrors/Err2Code.h>

/*
 * Hint: Set the Visual Studio project property Debugging > Working Directory
 * to $(OutDir), so that TestDeviceModule.osdev can be found.
 * The Working Directory setting is stored in the .vcxproj.user file, so it
 * is not stored in Git and needs to be set in each working copy.
 */

int main()
{
	if (!OSc_CheckVersion())
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
	OSc_SetDeviceModuleSearchPaths(paths);

	size_t count;
	OSc_RichError *err;
	if (OSc_CHECK_ERROR(err, OSc_GetNumberOfAvailableDevices(&count)))
	{
		fprintf(stderr, "Could not get count of devices\n");
		return 1;
	}

	printf("Count of devices = %zu\n", count);

	OSc_Device **devices;
	if (OSc_CHECK_ERROR(err, OSc_GetAllDevices(&devices, &count)))
	{
		fprintf(stderr, "Could not get all devices\n");
		return 1;
	}

	for (size_t i = 0; i < count; ++i) {
		const char *name;
		OSc_Device_GetName(devices[i], &name);
		printf("Device %zu: %s\n", i, name);
	}

	return 0;
}
