/*
 * OpenScan C API
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OPENSCANLIB_EXPORTS
#	ifdef _MSC_VER
#		define OSc_API __declspec(dllexport)
#	else
#		define OSc_API
#		error
#	endif
#else
#	ifdef _MSC_VER
#		define OSc_API __declspec(dllimport)
#	else
#		define OSc_API
#		error
#	endif
#endif


/**
 * \brief Maximum length for fixed-length string buffers.
 *
 * Note that the buffer size must be OSc_MAX_STR_LEN + 1.
 */
#define OSc_MAX_STR_LEN 511


typedef int32_t OSc_Log_Level;
enum
{
	OSc_Log_Level_Debug,
	OSc_Log_Level_Info,
	OSc_Log_Level_Warning,
	OSc_Log_Level_Error,
};


typedef int32_t OSc_Error;
enum
{
	OSc_Error_OK,
	OSc_Error_Unknown = 10000,
	OSc_Error_Unsupported_Operation,
	OSc_Error_Illegal_Argument,
	OSc_Error_Device_Module_Already_Exists,
	OSc_Error_No_Such_Device_Module,
	OSc_Error_Driver_Not_Available,
	OSc_Error_Device_Already_Open,
	OSc_Error_Device_Not_Opened_For_LSM,
	OSc_Error_Device_Does_Not_Support_Scanner,
	OSc_Error_Device_Does_Not_Support_Detector,
	OSc_Error_Wrong_Value_Type,
	OSc_Error_Setting_Not_Writable,
	OSc_Error_Wrong_Constraint_Type,
	OSc_Error_Unknown_Enum_Value_Name,
	OSc_Error_Acquisition_Running,
	OSc_Error_Not_Armed,
	OSc_Error_Waveform_Out_Of_Range,
	OSc_Error_Waveform_Memory_Size_Mismatch,
	OSc_Error_Data_Left_In_Fifo_After_Reading_Image,
};


typedef int32_t OSc_Error_Domain;
enum
{
	OSc_Error_Domain_OpenScanLib,
	OSc_Error_Domain_DAQmx,
	OSc_Error_Domain_NiFpga,
};


#define OSc_Check_Error(err, call) \
	((err = (call)) != OSc_Error_OK)

#define OSc_Return_If_Error(call) \
	do { \
		OSc_Error err; \
		if (OSc_Check_Error(err, (call))) \
			return err; \
	} while (0)


// TODO Use this as new error handling mechanism
// (functions return bool; last param is OSc_Error_Info **)
typedef struct OSc_Error_Info
{
	OSc_Error_Domain domain;
	int32_t code;
	char message[1024];
	struct OSc_Error_Info *cause;
} OSc_Error_Info;


typedef int32_t OSc_Value_Type;
enum
{
	OSc_Value_Type_String,
	OSc_Value_Type_Bool,
	OSc_Value_Type_Int32,
	OSc_Value_Type_Float64,
	OSc_Value_Type_Enum,
};


typedef int32_t OSc_Value_Constraint;
enum
{
	OSc_Value_Constraint_None,
	OSc_Value_Constraint_Discrete_Values,
	OSc_Value_Constraint_Range,
};


typedef int32_t OSc_Trigger_Source;
enum
{
	OSc_Trigger_Source_Scanner,  ///< Scanner generates trigger for detector
	OSc_Trigger_Source_Detector, ///< Detector generates trigger for scanner
	OSc_Trigger_Source_External, ///< Both scanner and detector are driven by an external trigger
};


typedef struct OSc_LSM OSc_LSM;
typedef struct OSc_Device OSc_Device;
typedef struct OSc_Scanner OSc_Scanner;
typedef struct OSc_Detector OSc_Detector;
typedef struct OSc_Setting OSc_Setting;
typedef struct OSc_Acquisition OSc_Acquisition;

typedef void (*OSc_Log_Func)(const char *message, OSc_Log_Level level, void *data);

// Returns true normally, or false to halt the acquisition
typedef bool (*OSc_Frame_Callback)(OSc_Acquisition *acq, uint32_t channel, void *pixels, void *data);


void OSc_API OSc_Set_Log_Func(OSc_Log_Func func, void *data);
void OSc_API OSc_Log_Set_Device_Log_Func(OSc_Device *device, OSc_Log_Func func, void *data);

void OSc_API OSc_DeviceModule_Set_Search_Paths(char **paths);

OSc_Error OSc_API OSc_LSM_Create(OSc_LSM **lsm);
OSc_Error OSc_API OSc_LSM_Destroy(OSc_LSM *lsm);
OSc_Error OSc_API OSc_LSM_Get_Scanner(OSc_LSM *lsm, OSc_Scanner **scanner);
OSc_Error OSc_API OSc_LSM_Set_Scanner(OSc_LSM *lsm, OSc_Scanner *scanner);
OSc_Error OSc_API OSc_LSM_Get_Detector(OSc_LSM *lsm, OSc_Detector **detector);
OSc_Error OSc_API OSc_LSM_Set_Detector(OSc_LSM *lsm, OSc_Detector *detector);
OSc_Error OSc_API OSc_LSM_Is_Running_Acquisition(OSc_LSM *lsm, bool *isRunning);

OSc_Error OSc_API OSc_Devices_Get_All(OSc_Device ***devices, size_t *count);
OSc_Error OSc_API OSc_Devices_Get_Count(size_t *count);

OSc_Error OSc_API OSc_Device_Get_Name(OSc_Device *device, const char **name);
OSc_Error OSc_API OSc_Device_Get_Display_Name(OSc_Device *device, const char **name);
OSc_Error OSc_API OSc_Device_Open(OSc_Device *device, OSc_LSM *lsm);
OSc_Error OSc_API OSc_Device_Close(OSc_Device *device);
OSc_Error OSc_API OSc_Device_Has_Scanner(OSc_Device *device, bool *hasScanner);
OSc_Error OSc_API OSc_Device_Has_Detector(OSc_Device *device, bool *hasDetector);
OSc_Error OSc_API OSc_Device_Get_Scanner(OSc_Device *device, OSc_Scanner **scanner);
OSc_Error OSc_API OSc_Device_Get_Detector(OSc_Device *device, OSc_Detector **detector);
OSc_Error OSc_API OSc_Device_Get_Settings(OSc_Device *device, OSc_Setting ***settings, size_t *count);
OSc_Error OSc_API OSc_Device_Get_Allowed_Resolutions(OSc_Device *device,
	size_t **widths, size_t **heights, size_t *count);
OSc_Error OSc_API OSc_Device_Get_Resolution(OSc_Device *device, size_t *width, size_t *height);
OSc_Error OSc_API OSc_Device_Set_Resolution(OSc_Device *device, size_t width, size_t height);
OSc_Error OSc_API OSc_Device_Get_Magnification(OSc_Device *device, double *magnification);
OSc_Error OSc_API OSc_Device_Set_Magnification(OSc_Device *device);

OSc_Error OSc_API OSc_Scanner_Get_Device(OSc_Scanner *scanner, OSc_Device **device);

OSc_Error OSc_API OSc_Detector_Get_Device(OSc_Detector *detector, OSc_Device **device);
OSc_Error OSc_API OSc_Detector_Get_Image_Size(OSc_Detector *detector, uint32_t *width, uint32_t *height);
OSc_Error OSc_API OSc_Detector_Get_Number_Of_Channels(OSc_Detector *detector, uint32_t *nChannels);
OSc_Error OSc_API OSc_Detector_Get_Bytes_Per_Sample(OSc_Detector *detector, uint32_t *bytesPerSample);

OSc_Error OSc_API OSc_Setting_Get_Name(OSc_Setting *setting, char *name);
OSc_Error OSc_API OSc_Setting_Get_Value_Type(OSc_Setting *setting, OSc_Value_Type *valueType);
OSc_Error OSc_API OSc_Setting_Is_Enabled(OSc_Setting *setting, bool *enabled);
OSc_Error OSc_API OSc_Setting_Is_Writable(OSc_Setting *setting, bool *writable);
OSc_Error OSc_API OSc_Setting_Get_Numeric_Constraint_Type(OSc_Setting *setting, OSc_Value_Constraint *constraintType);

OSc_Error OSc_API OSc_Setting_Get_String_Value(OSc_Setting *setting, char *value);
OSc_Error OSc_API OSc_Setting_Set_String_Value(OSc_Setting *setting, const char *value);

OSc_Error OSc_API OSc_Setting_Get_Bool_Value(OSc_Setting *setting, bool *value);
OSc_Error OSc_API OSc_Setting_Set_Bool_Value(OSc_Setting *setting, bool value);

OSc_Error OSc_API OSc_Setting_Get_Int32_Value(OSc_Setting *setting, int32_t *value);
OSc_Error OSc_API OSc_Setting_Set_Int32_Value(OSc_Setting *setting, int32_t value);
OSc_Error OSc_API OSc_Setting_Get_Int32_Range(OSc_Setting *setting, int32_t *min, int32_t *max);
OSc_Error OSc_API OSc_Setting_Get_Int32_Discrete_Values(OSc_Setting *setting, int32_t **values, size_t *count);

OSc_Error OSc_API OSc_Setting_Get_Float64_Value(OSc_Setting *setting, double *value);
OSc_Error OSc_API OSc_Setting_Set_Float64_Value(OSc_Setting *setting, double value);
OSc_Error OSc_API OSc_Setting_Get_Float64_Range(OSc_Setting *setting, double *min, double *max);
OSc_Error OSc_API OSc_Setting_Get_Float64_Discrete_Values(OSc_Setting *setting, double **values, size_t *count);

OSc_Error OSc_API OSc_Setting_Get_Enum_Value(OSc_Setting *setting, uint32_t *value);
OSc_Error OSc_API OSc_Setting_Set_Enum_Value(OSc_Setting *setting, uint32_t value);
OSc_Error OSc_API OSc_Setting_Get_Enum_Num_Values(OSc_Setting *setting, uint32_t *count);
OSc_Error OSc_API OSc_Setting_Get_Enum_Name_For_Value(OSc_Setting *setting, uint32_t value, char *name);
OSc_Error OSc_API OSc_Setting_Get_Enum_Value_For_Name(OSc_Setting *setting, uint32_t *value, const char *name);

OSc_Error OSc_API OSc_Acquisition_Create(OSc_Acquisition **acq, OSc_LSM *lsm);
OSc_Error OSc_API OSc_Acquisition_Destroy(OSc_Acquisition *acq);
OSc_Error OSc_API OSc_Acquisition_Set_Number_Of_Frames(OSc_Acquisition *acq, uint32_t numberOfFrames);
OSc_Error OSc_API OSc_Acquisition_Set_Trigger_Source(OSc_Acquisition *acq, OSc_Trigger_Source source);
OSc_Error OSc_API OSc_Acquisition_Set_Frame_Callback(OSc_Acquisition *acq, OSc_Frame_Callback callback);
OSc_Error OSc_API OSc_Acquisition_Get_Data(OSc_Acquisition *acq, void **data);
OSc_Error OSc_API OSc_Acquisition_Set_Data(OSc_Acquisition *acq, void *data);
OSc_Error OSc_API OSc_Acquisition_Arm(OSc_Acquisition *acq);
OSc_Error OSc_API OSc_Acquisition_Start(OSc_Acquisition *acq);
OSc_Error OSc_API OSc_Acquisition_Stop(OSc_Acquisition *acq);
OSc_Error OSc_API OSc_Acquisition_Wait(OSc_Acquisition *acq);


#ifdef __cplusplus
} // extern "C"
#endif
