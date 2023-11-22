#include "Module.h"
#include "InternalErrors.h"

#include <ss8str.h>

/*
 * This file contains platform-dependent utility functions for finding and
 * loading modules (DLLs).
 */

// Free a file list returned by OScInternal_FileList_Create()
void OScInternal_FileList_Free(ss8str *files) {
    ss8str *f = files;
    while (!ss8_is_empty(f))
        ss8_destroy(f++);
    ss8_destroy(f); // Sentinel
    free(files);
}

// Finds all fils under 'path' that have 'suffix'.
// Allocates array and element strings and places into 'files'.
OSc_RichError *OScInternal_FileList_Create(ss8str **files, const char *path,
                                           const char *suffix) {
    // Windows implementation for now

    DWORD err;

    size_t fileCount = 0;
    size_t fileCap = 16;
    *files = malloc(sizeof(ss8str) * fileCap);

    ss8str pattern;
    ss8_init_copy_cstr(&pattern, path);
    ss8_cat_cstr(&pattern, "/*");
    ss8_cat_cstr(&pattern, suffix);

    WIN32_FIND_DATAA findFileData;
    HANDLE findHandle = FindFirstFileA(ss8_cstr(&pattern), &findFileData);
    ss8_destroy(&pattern);
    if (findHandle == INVALID_HANDLE_VALUE) {
        err = GetLastError();
        ss8_init(&(*files)[0]); // Sentinel
        if (err == ERROR_FILE_NOT_FOUND)
            return OSc_OK;
        goto error;
    }

    for (;;) {
        if (fileCap - fileCount < 1) {
            size_t newFileCap = fileCap * 2;
            *files = realloc(*files, sizeof(ss8str) * newFileCap);
            fileCap = newFileCap;
        }
        ss8_init_copy_cstr(&(*files)[fileCount++], findFileData.cFileName);

        BOOL ok = FindNextFileA(findHandle, &findFileData);
        if (!ok) {
            err = GetLastError();
            FindClose(findHandle);
            ss8_init(&(*files)[fileCount]); // Sentinel
            if (err == ERROR_NO_MORE_FILES)
                return OSc_OK;
            goto error;
        }
    }

error:
    OScInternal_FileList_Free(*files);
    *files = NULL;
    return OScInternal_Error_Create("Failed to list files");
}

OSc_RichError *OScInternal_Module_Load(OScInternal_Module *module,
                                       const char *path) {
    *module = LoadLibraryA(path);
    if (*module == NULL)
        return OScInternal_Error_Unknown();
    return OSc_OK;
}

OSc_RichError *OScInternal_Module_GetEntryPoint(OScInternal_Module module,
                                                const char *funcName,
                                                void **func) {
    *func = (void *)GetProcAddress(module, funcName);
    if (!*func)
        return OScInternal_Error_Unknown();
    return OSc_OK;
}

bool OScInternal_Module_SupportsRichErrors(OScDev_ModuleImpl *modImpl) {
    return modImpl->supportsRichErrors;
}
