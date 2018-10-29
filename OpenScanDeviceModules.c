#include "OpenScanDeviceModules.h"

#include "Modules.h"
#include "OpenScanDeviceImpl.h"


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


#define STRINGIFY_EXPANSION(s) STRINGIFY(s)
#define STRINGIFY(s) #s
#define OSc_ENTRY_POINT_FUNC_NAME_STRING STRINGIFY_EXPANSION(OSc_ENTRY_POINT_FUNC_NAME)


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
	OSc_Return_If_Error(LoadModuleLibrary(path, &module));
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


void OSc_DeviceModule_Set_Search_Paths(char **paths)
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


OSc_Error OSc_DeviceModule_Get_Count(size_t *count)
{
	if (!g_loadedAdapters)
		LoadAdapters();

	*count = g_loadedAdapterCount;
	return OSc_Error_OK;
}


OSc_Error OSc_DeviceModule_Get_Names(const char **modules, size_t *count)
{
	if (!g_loadedAdapters)
		LoadAdapters();

	size_t i;
	for (i = 0; i < *count && i < g_loadedAdapterCount; ++i)
		modules[i] = g_loadedAdapters[i].name;
	*count = i;
	return OSc_Error_OK;
}


OSc_Error OSc_DeviceModule_Get_Devices(const char *module, OSc_Device ***devices, size_t *count)
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

	OSc_EntryPointFunc entryPoint;
	OSc_Return_If_Error(GetEntryPoint(mod->handle, OSc_ENTRY_POINT_FUNC_NAME_STRING, &entryPoint));

	size_t implsSize = 16;
	struct OSc_Device_Impl **deviceImpls = malloc(sizeof(void *) * implsSize);
	size_t implsCount = 0;
	for (;;)
	{
		size_t count = implsSize;
		OSc_Return_If_Error(entryPoint(deviceImpls, &count));
		if (count < implsSize)
		{
			implsCount = count;
			break;
		}
		deviceImpls = realloc(deviceImpls, sizeof(void *) * (implsSize *= 2));
	}

	*devices = NULL;
	*count = 0;
	for (size_t i = 0; i < implsCount; ++i)
	{
		OSc_Device **implDevices;
		size_t deviceCount;
		OSc_Error err;
		if (OSc_Check_Error(err, deviceImpls[i]->GetInstances(&implDevices, &deviceCount)))
		{
			char msg[OSc_MAX_STR_LEN + 1] = "Cannot enumerate devics: ";
			const char *model = NULL;
			deviceImpls[i]->GetModelName(&model);
			strcat(msg, model ? model : "(unknown)");
			OSc_Log_Warning(NULL, msg);
			continue;
		}

		if (deviceCount == 0)
			continue;

		size_t oldCount = *count;
		if (!*devices)
		{
			*count = deviceCount;
			*devices = malloc(sizeof(void *) * deviceCount);
		}
		else
		{
			*count += deviceCount;
			*devices = realloc(*devices, sizeof(void *) * *count);
		}

		for (size_t j = 0; j < deviceCount; ++j)
			*devices[oldCount + j] = implDevices[j];
	}

	free(deviceImpls);
	return OSc_Error_OK;
}