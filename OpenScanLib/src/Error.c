#include "OpenScanLibPrivate.h"
#include <RichErrors/Err2Code.h>

static const char* OScInternal_Error_MessageForLegacyCode(OScDev_Error code);

RERR_ErrorMapPtr OScInternal_Error_Map()
{
	static RERR_ErrorMapPtr map = NULL;
	if (map == NULL) {
		RERR_ErrorMapConfig config = {
			.mapFailureCode = -2,
			.maxMappedCode = 40000,
			.minMappedCode = 20000,
			.noErrorCode = 0,
			.outOfMemoryCode = -1,
		};
		RERR_ErrorMap_Create(&map, &config);
	}
	return map;
}


// APIs for device modules
OSc_RichError *OScInternal_Error_RegisterCodeDomain(const char *domainName, RERR_CodeFormat codeFormat) 
{
	return RERR_Domain_Register(domainName, codeFormat);
}

OScDev_Error OScInternal_Error_ReturnAsCode(OScDev_RichError *error) 
{
	return RERR_ErrorMap_RegisterThreadLocal(OScInternal_Error_Map(), error);
}


// APIs for OpenScanLib
OSc_RichError *OScInternal_Error_RetrieveRichErrors(int32_t code) 
{
	return RERR_ErrorMap_RetrieveThreadLocal(OScInternal_Error_Map(), code);
}


OSc_RichError *OScInternal_Error_RetrieveFromDevice(OSc_Device *device, OScDev_Error code)
{	
	if (!code)
		return OSc_OK;
	if (OScInternal_Device_SupportsRichErrors(device)) {
		return OScInternal_Error_RetrieveRichErrors(code);
	}
	else {
		return OScInternal_Error_CreateWithCode(OScInternal_Error_LegacyCodeDomain(), code, OScInternal_Error_MessageForLegacyCode(code));
	}
}


OSc_RichError *OScInternal_Error_RetrieveFromSetting(OSc_Setting *setting, OScDev_Error code)
{
	if (!code)
		return OSc_OK;
	if (OScInternal_Setting_SupportsRichErrors(setting)) {
		return OScInternal_Error_RetrieveRichErrors(code);
	}
	else {
		return OScInternal_Error_CreateWithCode(OScInternal_Error_LegacyCodeDomain(), code, OScInternal_Error_MessageForLegacyCode(code));
	}
}


OSc_RichError *OScInternal_Error_RetrieveFromModule(OScDev_ModuleImpl *modImpl, OScDev_Error code)
{
	if (!code)
		return OSc_OK;
	if (OScInternal_Module_SupportsRichErrors(modImpl)) {
		return OScInternal_Error_RetrieveRichErrors(code);
	}
	else {
		return OScInternal_Error_CreateWithCode(OScInternal_Error_LegacyCodeDomain(), code, OScInternal_Error_MessageForLegacyCode(code));
	}
}


static const char* OScInternal_Error_MessageForLegacyCode(OScDev_Error code)
{
	switch (code)
	{
		case OScDev_OK:
			OScInternal_LogError(NULL, "OScInternal_Error_MessageForLegacyCode: code cannot be 0.");
			abort();
		case OScDev_Error_Unsupported_Operation:
			return "Unsupported Operation";
		case OScDev_Error_Illegal_Argument:
			return "Illegal Argument";
		case OScDev_Error_Device_Module_Already_Exists:
			return "Device Module Already Exists";
		case OScDev_Error_No_Such_Device_Module:
			return "No Such Device Module";
		case OScDev_Error_Driver_Not_Available:
			return "Driver Not Available";
		case OScDev_Error_Device_Already_Open:
			return "Device Already Open";
		case OScDev_Error_Device_Not_Opened_For_LSM:
			return "Device Not Opened For LSM";
		case OScDev_Error_Device_Does_Not_Support_Clock:
			return "Device Does Not Support Clock";
		case OScDev_Error_Device_Does_Not_Support_Scanner:
			return "Device Does Not Support Scanner";
		case OScDev_Error_Device_Does_Not_Support_Detector:
			return "Device Does Not Support Detector";
		case OScDev_Error_Wrong_Value_Type:
			return "Wrong Value Type";
		case OScDev_Error_Setting_Not_Writable:
			return "Setting Not Writable";
		case OScDev_Error_Wrong_Constraint_Type:
			return "Wrong Constraint Type";
		case OScDev_Error_Unknown_Enum_Value_Name:
			return "Unknown Enum Value Name";
		case OScDev_Error_Acquisition_Running:
			return "Acquisition Running";
		case OScDev_Error_Not_Armed:
			return "Not Armed";
		case OScDev_Error_Waveform_Out_Of_Range:
			return "Waveform Out Of Range";
		case OScDev_Error_Waveform_Memory_Size_Mismatch:
			return "Waveform Memory Size Mismatch";
		case OScDev_Error_Data_Left_In_Fifo_After_Reading_Image:
			return "Data Left In Fifo After Reading Image";
		case OScDev_Error_Unknown:
		default:
			return "Unknown Error";
	}
}




OSc_RichError *OScInternal_Error_AsRichError(OScDev_Error code)
{
	return OScInternal_Error_CreateWithCode(OScInternal_Error_LegacyCodeDomain(), code, OScInternal_Error_MessageForLegacyCode(code));
}


OSc_RichError *OScInternal_Error_Create(const char *message)
{
	return RERR_Error_Create(message);
}


OSc_RichError *OScInternal_Error_CreateWithCode(const char *domainName, int32_t code, const char *message)
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


char *OScInternal_Error_LegacyCodeDomain() 
{
	static char *domainName = NULL;
	if (domainName == NULL) {
		domainName = "OpenScan legacy device module";
		RERR_Domain_Register(domainName, RERR_CodeFormat_I32);
	}
	return domainName;
}



const char *OScInternal_Error_GetMessage(OSc_RichError *error)
{
	return RERR_Error_GetMessage(error);
}


const char* OSc_Error_GetMessage(OSc_RichError* error)
{
	return OScInternal_Error_GetMessage(error);
}


const char *OScInternal_Error_GetDomain(OSc_RichError *error)
{
	return RERR_Error_GetDomain(error);
}


const char* OSc_Error_GetDomain(OSc_RichError* error)
{
	return OScInternal_Error_GetDomain(error);
}


int32_t OScInternal_Error_GetCode(OSc_RichError *error)
{
	return RERR_Error_GetCode(error);
}


int32_t OSc_Error_GetCode(OSc_RichError* error)
{
	return OScInternal_Error_GetCode(error);
}


OSc_RichError *OScInternal_Error_GetCause(OSc_RichError *error)
{
	return RERR_Error_GetCause(error);
}


OSc_RichError* OSc_Error_GetCause(OSc_RichError* error)
{
	return OScInternal_Error_GetCause(error);
}


const char* OScInternal_Error_Format(OSc_RichError* error)
{
	char* format = (char*)malloc(1000 * sizeof(char));
	sprintf(format, "<%s> (<%s> error <%d>)", RERR_Error_GetMessage(error),
		RERR_Error_GetDomain(error), RERR_Error_GetCode(error));
	return format;
}


const char* OSc_Error_Format(OSc_RichError* error)
{
	return OScInternal_Error_Format(error);
}


const char* OScInternal_Error_FormatRecursive(OSc_RichError* error)
{
	char* format = (char*)malloc(1000 * sizeof(char));
	OSc_RichError *cause = OSc_Error_GetCause(error);
	if (cause) 
		sprintf(format, "<%s> (<%s> error <%d>)[caused by:%s]", RERR_Error_GetMessage(error),
			RERR_Error_GetDomain(error), RERR_Error_GetCode(error), OSc_Error_FormatRecursive(cause));
	else
		sprintf(format, "<%s> (<%s> error <%d>)", RERR_Error_GetMessage(error),
			RERR_Error_GetDomain(error), RERR_Error_GetCode(error));
	return format;
}


const char* OSc_Error_FormatRecursive(OSc_RichError* error)
{
	return OScInternal_Error_FormatRecursive(error);
}


void OScInternal_Error_Destroy(OSc_RichError *error)
{
	RERR_Error_Destroy(error);
}


void OSc_Error_Destroy(OSc_RichError* error)
{
	OScInternal_Error_Destroy(error);
}
