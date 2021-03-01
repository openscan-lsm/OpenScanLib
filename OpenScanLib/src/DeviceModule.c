#include "DeviceInterface.h"
#include "Module.h"
#include "InternalErrors.h"

#define OScDevInternal_BUILDING_OPENSCANLIB 1
#include "OpenScanDeviceLibPrivate.h"



// Array of strings; each element _and_ the array itself must be freed when
// replacing; last element is always null
static char **g_adapterPaths;

// Array of valid module handles
struct Module
{
	OScInternal_Module handle;
	char *name;
};
static struct Module *g_loadedAdapters;
static size_t g_loadedAdapterCount;
static size_t g_loadedAdaptersCap;


static OSc_RichError *LoadAdapter(const char *path, const char *name)
{
	if (!g_loadedAdapters)
	{
		g_loadedAdaptersCap = 16;
		g_loadedAdapters = malloc(sizeof(struct Module) * g_loadedAdaptersCap);
	}

	// Ensure we do not already have a module with 'name'
	// TODO To support a large number of modules, this should be made more
	// efficient (binary search?).
	for (size_t i = 0; i < g_loadedAdapterCount; ++i)
	{
		if (strcmp(g_loadedAdapters[i].name, name) == 0)
			return OScInternal_Error_DeviceModuleAlreadyExists();
	}

	OScInternal_Module module;
	OSc_RichError *err;
	if (OSc_CHECK_ERROR(err, OScInternal_Module_Load(&module, path)))
		return err;
	if (g_loadedAdapterCount == g_loadedAdaptersCap)
	{
		g_loadedAdapters = realloc(g_loadedAdapters,
			sizeof(struct Module) * (g_loadedAdaptersCap *= 2));
	}

	struct Module *desc = &g_loadedAdapters[g_loadedAdapterCount++];
	desc->handle = module;
	desc->name = malloc(strlen(name) + 1);
	strcpy(desc->name, name);

	return OSc_OK;
}


static void LoadAdaptersAtPath(const char *path)
{
	char **files;
	OScInternal_FileList_Create(&files, path, ".osdev");
	for (char **pfile = files; *pfile; ++pfile)
	{
		char filePath[512];
		strncpy(filePath, path, sizeof(filePath) - 1);
		strncat(filePath, "/", sizeof(filePath) - 1 - strlen(filePath));
		strncat(filePath, *pfile, sizeof(filePath) - 1 - strlen(filePath));
		char name[512];
		strncpy(name, *pfile, sizeof(name) - 1);
		// Remove suffix (we trust OScInternal_FileList_Create returned what it should)
		*strrchr(name, '.') = '\0';
		OSc_RichError *err = LoadAdapter(filePath, name);
		// TODO Log or report error
	}
	OScInternal_FileList_Free(files);
}


static void LoadAdapters()
{
	if (!g_adapterPaths)
	{
		// Default to no paths
		g_adapterPaths = malloc(sizeof(void *));
		g_adapterPaths[0] = NULL;
	}

	for (const char **p = g_adapterPaths; *p; ++p)
		LoadAdaptersAtPath(*p);
}


static void FreeAdapterPaths()
{
	if (g_adapterPaths)
	{
		char **p = g_adapterPaths;
		while (*p++)
			free(*p);

		free(g_adapterPaths);
		g_adapterPaths = NULL;
	}
}


void OSc_SetDeviceModuleSearchPaths(char **paths)
{
	FreeAdapterPaths();

	size_t nPaths = 0;
	char **p = paths;
	while (*p++)
		++nPaths;

	g_adapterPaths = malloc(sizeof(void *) * (nPaths + 1));
	p = paths;
	char **q = g_adapterPaths;
	while (*p)
	{
		char *path = malloc(strlen(*p) + 1);
		strcpy(path, *p++);
		*q++ = path;
	}
	*q = NULL;
}


OSc_RichError *OScInternal_DeviceModule_GetCount(size_t *count)
{
	if (!g_loadedAdapters)
		LoadAdapters();

	*count = g_loadedAdapterCount;
	return OSc_OK;
}


OSc_RichError *OScInternal_DeviceModule_GetNames(const char **modules, size_t *count)
{
	if (!g_loadedAdapters)
		LoadAdapters();

	size_t i;
	for (i = 0; i < *count && i < g_loadedAdapterCount; ++i)
		modules[i] = g_loadedAdapters[i].name;
	*count = i;
	return OSc_OK;
}


OSc_RichError *OScInternal_DeviceModule_GetDeviceImpls(const char *module, OScInternal_PtrArray **deviceImpls)
{
	struct Module *mod = NULL;
	for (size_t i = 0; i < g_loadedAdapterCount; ++i)
	{
		if (strcmp(g_loadedAdapters[i].name, module) == 0)
		{
			mod = &g_loadedAdapters[i];
			break;
		}
	}
	if (!mod)
		return OScInternal_Error_NoSuchDeviceModule();

	OScDevInternal_EntryPointPtr entryPoint;
	OSc_RichError *err;
	OScDev_Error errCode;
	if (OSc_CHECK_ERROR(err, OScInternal_Module_GetEntryPoint(mod->handle, OScDevInternal_ENTRY_POINT_NAME, (void *)&entryPoint)))
		return err;

	struct OScDevInternal_Interface **funcTablePtr;
	OScDev_ModuleImpl *modImpl;
	uint32_t dpiVersion = entryPoint(&funcTablePtr, &modImpl);
	if (dpiVersion != OScDevInternal_ABI_VERSION)
	{
		// TODO We could support non-exact version matches.
		return OScInternal_Error_Unknown();
	}

	*funcTablePtr = &DeviceInterfaceFunctionTable;

	if (modImpl->Open)
	{
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
