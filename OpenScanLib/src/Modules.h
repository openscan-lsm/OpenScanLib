#pragma once

#include "OpenScanLibPrivate.h"

#ifndef _WIN32
#error Only Windows version implemented at this time.
#endif

#include <Windows.h>


typedef HMODULE OSc_Module_Handle;

typedef OSc_Error(*OSc_EntryPointFunc)(struct OSc_Device_Impl **, size_t *);

void FreeFileList(char **files);

OSc_Error FindFilesWithSuffix(const char *path, const char *suffix,
	char ***files);

OSc_Error LoadModuleLibrary(const char *path, OSc_Module_Handle *module);

OSc_Error GetEntryPoint(OSc_Module_Handle module, const char *funcName, OSc_EntryPointFunc *func);