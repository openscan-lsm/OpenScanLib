#include "DeviceInterface.h"
#include "InternalErrors.h"
#include "Module.h"

#define OScDevInternal_BUILDING_OPENSCANLIB 1
#include "OpenScanDeviceLibPrivate.h"

#include <ss8str.h>

// Array of strings; each element _and_ the array itself must be freed when
// replacing; last element is always empty string.
static ss8str *g_adapterPaths;

// Array of valid module handles
struct Module {
    OScInternal_Module handle;
    ss8str name;
};
static struct Module *g_loadedAdapters;
static size_t g_loadedAdapterCount;
static size_t g_loadedAdaptersCap;

static OSc_RichError *LoadAdapter(const char *path, const char *name) {
    if (!g_loadedAdapters) {
        g_loadedAdaptersCap = 16;
        g_loadedAdapters = malloc(sizeof(struct Module) * g_loadedAdaptersCap);
    }

    // Ensure we do not already have a module with 'name'
    // TODO To support a large number of modules, this should be made more
    // efficient (binary search?).
    for (size_t i = 0; i < g_loadedAdapterCount; ++i) {
        if (ss8_equals_cstr(&g_loadedAdapters[i].name, name))
            return OScInternal_Error_DeviceModuleAlreadyExists();
    }

    OScInternal_Module module;
    OSc_RichError *err;
    if (OSc_CHECK_ERROR(err, OScInternal_Module_Load(&module, path)))
        return err;
    if (g_loadedAdapterCount == g_loadedAdaptersCap) {
        g_loadedAdapters =
            realloc(g_loadedAdapters,
                    sizeof(struct Module) * (g_loadedAdaptersCap *= 2));
    }

    struct Module *desc = &g_loadedAdapters[g_loadedAdapterCount++];
    desc->handle = module;
    ss8_init_copy_cstr(&desc->name, name);

    return OSc_OK;
}

static void LoadAdaptersAtPath(const ss8str *path) {
    ss8str *files;
    OScInternal_FileList_Create(&files, ss8_cstr(path), ".osdev");
    if (!files)
        return;
    for (ss8str *pfile = files; !ss8_is_empty(pfile); ++pfile) {
        ss8str filePath;
        ss8_init_copy(&filePath, path);
        ss8_cat_ch(&filePath, '/');
        ss8_cat(&filePath, pfile);

        ss8str name;
        ss8_init_copy(&name, pfile);
        // Remove suffix (we trust OScInternal_FileList_Create returned what it
        // should)
        size_t dot_pos = ss8_rfind_ch(&name, ss8_len(&name), '.');
        ss8_substr_inplace(&name, 0, dot_pos);

        OSc_RichError *err = LoadAdapter(ss8_cstr(&filePath), ss8_cstr(&name));
        ss8_destroy(&name);
        ss8_destroy(&filePath);

        ss8str msg;
        ss8_init(&msg);
        ss8_set_len(&msg, 1024);
        OScInternal_Error_FormatRecursive(err, ss8_mutable_cstr(&msg),
                                          ss8_len(&msg) + 1);
        ss8_set_len_to_cstrlen(&msg);
        OScInternal_Error_Destroy(err);
        OScInternal_LogError(NULL, ss8_cstr(&msg));
        ss8_destroy(&msg);
    }
    OScInternal_FileList_Free(files);
}

static void LoadAdapters() {
    if (!g_adapterPaths) {
        // Default to no paths
        g_adapterPaths = malloc(sizeof(ss8str));
        ss8_init(&g_adapterPaths[0]);
    }

    for (const ss8str *p = g_adapterPaths; !ss8_is_empty(p); ++p)
        LoadAdaptersAtPath(p);
}

static void FreeAdapterPaths() {
    if (g_adapterPaths) {
        ss8str *p = g_adapterPaths;
        while (!ss8_is_empty(p))
            ss8_destroy(p++);
        ss8_destroy(p); // Sentinel

        free(g_adapterPaths);
        g_adapterPaths = NULL;
    }
}

void OSc_SetDeviceModuleSearchPaths(const char **paths) {
    FreeAdapterPaths();

    size_t nPaths = 0;
    const char **p = paths;
    while (*p++)
        ++nPaths;

    g_adapterPaths = malloc(sizeof(ss8str) * (nPaths + 1));
    for (size_t i = 0; i < nPaths; ++i) {
        ss8_init_copy_cstr(&g_adapterPaths[i], paths[i]);
    }
    ss8_init(&g_adapterPaths[nPaths]); // Sentinel
}

OSc_RichError *OScInternal_DeviceModule_GetCount(size_t *count) {
    if (!g_loadedAdapters)
        LoadAdapters();

    *count = g_loadedAdapterCount;
    return OSc_OK;
}

OSc_RichError *OScInternal_DeviceModule_GetNames(const char **modules,
                                                 size_t *count) {
    if (!g_loadedAdapters)
        LoadAdapters();

    size_t i;
    for (i = 0; i < *count && i < g_loadedAdapterCount; ++i)
        modules[i] = ss8_cstr(&g_loadedAdapters[i].name);
    *count = i;
    return OSc_OK;
}

OSc_RichError *
OScInternal_DeviceModule_GetDeviceImpls(const char *module,
                                        OScInternal_PtrArray **deviceImpls) {
    struct Module *mod = NULL;
    for (size_t i = 0; i < g_loadedAdapterCount; ++i) {
        if (ss8_equals_cstr(&g_loadedAdapters[i].name, module)) {
            mod = &g_loadedAdapters[i];
            break;
        }
    }
    if (!mod)
        return OScInternal_Error_NoSuchDeviceModule();

    OScDevInternal_EntryPointPtr entryPoint;
    OSc_RichError *err;
    OScDev_Error errCode;
    if (OSc_CHECK_ERROR(err, OScInternal_Module_GetEntryPoint(
                                 mod->handle, OScDevInternal_ENTRY_POINT_NAME,
                                 (void *)&entryPoint)))
        return err;

    struct OScDevInternal_Interface **funcTablePtr;
    OScDev_ModuleImpl *modImpl;
    uint32_t dpiVersion = entryPoint(&funcTablePtr, &modImpl);
    if (dpiVersion != OScDevInternal_ABI_VERSION) {
        // TODO We could support non-exact version matches.
        return OScInternal_Error_Unknown();
    }

    *funcTablePtr = &DeviceInterfaceFunctionTable;

    if (modImpl->Open) {
        errCode = modImpl->Open();
        if (errCode)
            return OScInternal_Error_RetrieveFromModule(modImpl, errCode);
    }
    // TODO We need to also call Close() when shutting down

    errCode = modImpl->GetDeviceImpls(deviceImpls);
    if (errCode)
        return OScInternal_Error_RetrieveFromModule(modImpl, errCode);
    return OSc_OK;
}
