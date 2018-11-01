#include "Modules.h"

#include <string.h>


/*
 * This file contains platform-dependent utility functions for finding and
 * loading modules (DLLs).
 */


// Free a file list returned by FindFilesWithSuffix()
void FreeFileList(char **files)
{
	for (char **pfile = files; *pfile; ++pfile)
		free(*pfile);
	free(files);
}


// Finds all fils under 'path' that have 'suffix'.
// Allocates array and element strings and places into 'files'.
OSc_Error FindFilesWithSuffix(const char *path, const char *suffix,
	char ***files)
{
	// Windows implementation for now

	DWORD err;

	size_t fileCount = 0;
	size_t fileCap = 16;
	*files = malloc(sizeof(void *) * fileCap);
	memset(*files, 0, fileCap);

	char pattern[512];
	strncpy(pattern, path, sizeof(pattern) - 1);
	strncat(pattern, "/*", sizeof(pattern) - 1 - strlen(pattern));
	strncat(pattern, suffix, sizeof(pattern) - 1 - strlen(pattern));

	WIN32_FIND_DATAA findFileData;
	HANDLE findHandle = FindFirstFileA(pattern, &findFileData);
	if (findHandle == INVALID_HANDLE_VALUE)
	{
		err = GetLastError();
		if (err == ERROR_FILE_NOT_FOUND)
			return OSc_Error_OK;
		goto error;
	}
	for (;;)
	{
		size_t nameLen = strlen(findFileData.cFileName);
		char *name = malloc(nameLen + 1);
		strcpy(name, findFileData.cFileName);

		if (fileCap - fileCount < 1)
		{
			size_t newFileCap = fileCap * 2;
			*files = realloc(*files, sizeof(void *) * newFileCap);
			memset(*files + fileCap, 0, newFileCap - fileCap);
			fileCap = newFileCap;
		}
		(*files)[fileCount++] = name;

		BOOL ok = FindNextFileA(findHandle, &findFileData);
		if (!ok)
		{
			err = GetLastError();
			FindClose(findHandle);
			if (err == ERROR_NO_MORE_FILES)
			{
				(*files)[fileCount] = NULL;
				return OSc_Error_OK;
			}
			goto error;
		}
	}
	return OSc_Error_OK;

error:
	FreeFileList(*files);
	*files = NULL;
	return err;
}


OSc_Error LoadModuleLibrary(const char *path, OSc_Module_Handle *module)
{
	*module = LoadLibraryA(path);
	if (*module == NULL)
		return OSc_Error_Unknown; // TODO
	return OSc_Error_OK;
}


OSc_Error GetEntryPoint(OSc_Module_Handle module, const char *funcName, OSc_EntryPointFunc *func)
{
	*func = (OSc_EntryPointFunc) GetProcAddress(module, funcName);
	if (!*func)
		return OSc_Error_Unknown;
	return OSc_Error_OK;
}