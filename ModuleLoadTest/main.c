#include "OpenScanLib.h"

#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#endif

int main(int argc, char *argv[])
{
	if (!OSc_CheckVersion())
	{
		fprintf(stderr, "OpenScanLib ABI version mismatch\n");
		return 1;
	}

	if (argc != 2) {
		fprintf(stderr, "Expected 1 argument (module whose directory to test)\n");
		return 1;
	}

	printf("Using directory containing \"%s\" as search path\n", argv[1]);
	char dir[MAX_PATH + 1];
	strncpy(dir, argv[1], MAX_PATH);
	char *last_slash = strrchr(dir, '/');
	char *last_bslash = strrchr(dir, '\\');
	char *dirend = last_slash > last_bslash ? last_slash : last_bslash;
	*dirend = '\0';
	printf("Using directory \"%s\" as search path\n", dir);

	char *paths[2];
	paths[0] = dir;
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
	if (count < 1) {
		return 1;
	}

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
