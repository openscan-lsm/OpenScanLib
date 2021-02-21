#pragma once

#include "OpenScanLibPrivate.h"
#include <RichErrors/Err2Code.h>

static RERR_ErrorMapPtr map;


// APIs for device modules
OSc_RichError *OScInternal_Error_RegisterCodeDomain(const char *domainName, RERR_CodeFormat codeFormat) 
{
	return RERR_Domain_Register(domainName, codeFormat);
}

OScDev_Error OScInternal_Error_ReturnAsCode(OScDev_RichError *error) 
{
	return RERR_ErrorMap_RegisterThreadLocal(map, error);
}


// APIs for OpenScanLib
OSc_RichError *OScInternal_Error_RetrieveRichErrors(int32_t code) 
{
	return RERR_ErrorMap_RetrieveThreadLocal(map, code);
}


OSc_RichError* OScInternal_Error_Create(const char* message)
{
	return RERR_Error_Create(message);
}


OSc_RichError* OScInternal_Error_CreateWithCode(const char* domainName, int32_t code, const char* message)
{
	return RERR_Error_CreateWithCode(domainName, code, message);
}


OSc_RichError *OScInternal_Error_Wrap(OSc_RichError *cause, const char *message) 
{
	return RERR_Error_Wrap(cause, message);
}


OSc_RichError *OScInternal_Error_WrapWithCode(OSc_RichError *cause, const char *domainName, int32_t code, const char *message) 
{
	return RERR_Error_WrapWithCode(cause, domainName, code, message);
}


char *OScInternal_Error_OScDomain() 
{
	static char *domainName = NULL;
	if (domainName == NULL) {
		domainName = "OpenScan";
		RERR_Domain_Register(domainName, RERR_CodeFormat_I32);
	}
	return domainName;
}


char *OScInternal_Error_LegacyCodeDomain() 
{
	static char *domainName = NULL;
	if (domainName == NULL) {
		domainName = "OpenScan legacy device module";
		RERR_Domain_Register(domainName, RERR_CodeFormat_I32);
	}
	return domainName;
}


const char *OSc_Error_GetMessage(OSc_RichError *error) 
{
	return RERR_Error_GetMessage(error);
}


const char *OSc_Error_GetDomain(OSc_RichError *error) 
{
	return RERR_Error_GetDomain(error);
}


int32_t OSc_Error_GetCode(OSc_RichError *error) 
{
	return RERR_Error_GetCode(error);
}


OSc_RichError *OSc_Error_GetCause(OSc_RichError *error) 
{
	return RERR_Error_GetCause(error);
}


void OSc_Error_Destroy(OSc_RichError *error) 
{
	RERR_Error_Destroy(error);
}
