#pragma once

#include "OpenScanLibPrivate.h"

#ifndef _WIN32
#error Only Windows version implemented at this time.
#endif

#include <Windows.h>

typedef HMODULE OScInternal_Module;

void OScInternal_FileList_Free(char **files);

OSc_RichError *OScInternal_FileList_Create(char ***files, const char *path,
                                           const char *suffix);

OSc_RichError *OScInternal_Module_Load(OScInternal_Module *module,
                                       const char *path);

OSc_RichError *OScInternal_Module_GetEntryPoint(OScInternal_Module module,
                                                const char *funcName,
                                                void **func);
