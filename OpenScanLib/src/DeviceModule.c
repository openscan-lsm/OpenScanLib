#include "DeviceInterface.h"
#include "Modules.h"

#define OScDevInternal_BUILDING_OPENSCANLIB
#include "OpenScanDeviceLibPrivate.h"


// Array of strings; each element _and_ the array itself must be freed when
// replacing; last element is always null
static char **g_adapterPaths;

// Array of valid module handles
struct Module
{
	OSc_Module_Handle handle;
	char *name;
};
static struct Module *g_loadedAdapters;
static size_t g_loadedAdapterCount;
static size_t g_loadedAdaptersCap;


static OSc_Error LoadAdapter(const char *path, const char *name)
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
			return OSc_Error_Device_Module_Already_Exists;
	}

	OSc_Module_Handle module;
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, LoadModuleLibrary(path, &module)))
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

	return OSc_Error_OK;
}


static void LoadAdaptersAtPath(const char *path)
{
	char **files;
	FindFilesWithSuffix(path, ".osdev", &files);
	for (char **pfile = files; *pfile; ++pfile)
	{
		char filePath[512];
		strncpy(filePath, path, sizeof(filePath) - 1);
		strncat(filePath, "/", sizeof(filePath) - 1 - strlen(filePath));
		strncat(filePath, *pfile, sizeof(filePath) - 1 - strlen(filePath));
		char name[512];
		strncpy(name, *pfile, sizeof(name) - 1);
		// Remove suffix (we trust FindFilesWithSuffix returned what it should)
		*strrchr(name, '.') = '\0';
		OSc_Error err = LoadAdapter(filePath, name);
		// TODO Log or report error
	}
	FreeFileList(files);
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


OSc_Error OScInternal_DeviceModule_GetCount(size_t *count)
{
	if (!g_loadedAdapters)
		LoadAdapters();

	*count = g_loadedAdapterCount;
	return OSc_Error_OK;
}


OSc_Error OScInternal_DeviceModule_GetNames(const char **modules, size_t *count)
{
	if (!g_loadedAdapters)
		LoadAdapters();

	size_t i;
	for (i = 0; i < *count && i < g_loadedAdapterCount; ++i)
		modules[i] = g_loadedAdapters[i].name;
	*count = i;
	return OSc_Error_OK;
}


OSc_Error OScInternal_DeviceModule_GetDeviceImpls(const char *module, const OScDev_PtrArray **deviceImpls)
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
		return OSc_Error_No_Such_Device_Module;

	OScDevInternal_EntryPointPtr entryPoint;
	OSc_Error err;
	if (OSc_CHECK_ERROR(err, GetEntryPoint(mod->handle, OScDevInternal_ENTRY_POINT_NAME, (void *)&entryPoint)))
		return err;

	struct OScDevInternal_Interface **funcTablePtr;
	OScDev_ModuleImpl *modImpl;
	uint32_t dpiVersion = entryPoint(&funcTablePtr, &modImpl);
	if (dpiVersion != OScDevInternal_ABI_VERSION)
	{
		// TODO We could support non-exact version matches.
		return OSc_Error_Unknown;
	}

	*funcTablePtr = &DeviceInterfaceFunctionTable;

	if (modImpl->Open)
	{
		if (OSc_CHECK_ERROR(err, modImpl->Open()))
			return err;
	}
	// TODO We need to also call Close() when shutting down

	if (OSc_CHECK_ERROR(err, modImpl->GetDeviceImpls(deviceImpls)))
		return err;
	return OSc_Error_OK;
}