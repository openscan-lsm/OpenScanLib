#include "OpenScanLib.h"

#include <ss8str.h>

#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#endif

int main(int argc, char *argv[]) {
    if (!OSc_CheckVersion()) {
        fprintf(stderr, "OpenScanLib ABI version mismatch\n");
        return __LINE__;
    }

    if (argc != 2) {
        fprintf(stderr,
                "Expected 1 argument (module whose directory to test)\n");
        return __LINE__;
    }

    printf("Using directory containing \"%s\" as search path\n", argv[1]);
    ss8str dir;
    ss8_init_copy_cstr(&dir, argv[1]);
    size_t last_slash = ss8_rfind_ch(&dir, ss8_len(&dir), '/');
    last_slash = last_slash == 0 ? 1 : last_slash;
    last_slash = last_slash == SIZE_MAX ? 0 : last_slash;
    size_t last_bslash = ss8_rfind_ch(&dir, ss8_len(&dir), '\\');
    last_bslash = last_bslash == SIZE_MAX ? 0 : last_bslash;
    ss8_substr_inplace(&dir, 0,
                       last_slash > last_bslash ? last_slash : last_bslash);
    if (ss8_is_empty(&dir))
        ss8_copy_ch(&dir, '.');
    printf("Using directory \"%s\" as search path\n", ss8_cstr(&dir));

    const char *paths[2];
    paths[0] = ss8_cstr(&dir);
    paths[1] = NULL;
    OSc_SetDeviceModuleSearchPaths(paths);
    ss8_destroy(&dir);

    size_t count;
    OSc_RichError *err;
    if (OSc_CHECK_ERROR(err, OSc_GetNumberOfAvailableDevices(&count))) {
        fprintf(stderr, "Could not get count of devices\n");
        return __LINE__;
    }

    printf("Count of devices = %zu\n", count);
    if (count < 1) {
        return __LINE__;
    }

    OSc_Device **devices;
    if (OSc_CHECK_ERROR(err, OSc_GetAllDevices(&devices, &count))) {
        fprintf(stderr, "Could not get all devices\n");
        return __LINE__;
    }

    for (size_t i = 0; i < count; ++i) {
        const char *name;
        OSc_Device_GetName(devices[i], &name);
        printf("Device %zu: %s\n", i, name);
    }

    return 0;
}
