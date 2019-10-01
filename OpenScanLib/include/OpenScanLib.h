/**
 * \file
 * \brief OpenScan C API
 *
 * This header is to be included by applications using OpenScan.
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
 * \mainpage OpenScanLib: OpenScan C Application Programming Interface
 *
 * For API documentation, see \ref api.
 *
 * For partial documentation on OpenScanLib internals, see \ref internal.
 */

/**
 * \defgroup api OpenScan C API
 */

/**
 * \defgroup internal OpenScanLib Internals
 *
 * These are the internal workings of OpenScanLib; application code must not
 * use these declarations and definitions directly.
 */

/**
 * \ingroup internal
 * \sa OScInternal_ABI_VERSION
 */
#define OScInternal_MAKE_VERSION(major, minor) (((uint32_t)(major) << 16) | (uint16_t)(minor))

/**
 * \ingroup internal
 * \brief ABI version.
 *
 * This is the binary compatibility version of the application interface. It is
 * independent of the device module programming interface version. It also has
 * no connection to any release version number of OpenScanLib.
 *
 * The version number check must be done explicitly by the client application
 * (see OSc_CheckVersion()).
 *
 * Maintainers:
 *
 * The major version must be incremented when:
 * - A function is removed
 * - A function signature changes
 * - Behavior of a function changes in an incompatible way
 * - An enum constant is removed
 * - An enum constant that must be processed by client code is added
 *
 * The minor version must be incremented when:
 * - A function is added
 * - An enum constant is added, but need not be processed by client code
 * - A bug is fixed without changing the _intended_ behavior (severe cases
 *   may warrant a major version increment if workarounds may have been
 *   implemented in client applications)
 *
 * The above list is not comprehensive.
 */
#define OScInternal_ABI_VERSION OScInternal_MAKE_VERSION(1, 0)

/**
 * \addtogroup api
 * @{
 */

/**
 * \brief Maximum length for fixed-length string buffers.
 *
 * Note that the buffer size must be OSc_MAX_STR_LEN + 1.
 */
#define OSc_MAX_STR_LEN 511


typedef int32_t OSc_LogLevel;
enum
{
	OSc_LogLevel_Debug,
	OSc_LogLevel_Info,
	OSc_LogLevel_Warning,
	OSc_LogLevel_Error,
};


typedef int32_t OSc_Error;
enum
{
	OSc_Error_OK,

	// WARNING: These must match exactly the definitions of OScDev_Error_* in
	// OpenScanDeviceLib.h (normaly we would ensure that a common definition is
	// used, but these codes are temporary and will be replaced soon by a new
	// error handling mechanism).
	OSc_Error_Unknown = 10000,
	OSc_Error_Unsupported_Operation,
	OSc_Error_Illegal_Argument,
	OSc_Error_Device_Module_Already_Exists,
	OSc_Error_No_Such_Device_Module,
	OSc_Error_Driver_Not_Available,
	OSc_Error_Device_Already_Open,
	OSc_Error_Device_Not_Opened_For_LSM,
	OSc_Error_Device_Does_Not_Support_Clock,
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


/**
 * \brief Convenience macro for if-statement checking error.
 *
 * For example,
 *
 *     OSc_Error err;
 *     if (OSc_CHECK_ERROR(err, OSc_Foo(...))) {
 *         // Clean up resources
 *         return err;
 *     }
 */
#define OSc_CHECK_ERROR(err, call) \
	((err = (call)) != OSc_Error_OK)


typedef int32_t OSc_ValueType;
enum
{
	OSc_ValueType_String,
	OSc_ValueType_Bool,
	OSc_ValueType_Int32,
	OSc_ValueType_Float64,
	OSc_ValueType_Enum,
};


typedef int32_t OSc_ValueConstraint;
enum
{
	OSc_ValueConstraint_None,
	OSc_ValueConstraint_Discrete,
	OSc_ValueConstraint_Continuous,
};


typedef struct OScInternal_LSM OSc_LSM;
typedef struct OScInternal_Device OSc_Device;
typedef struct OScInternal_Clock OSc_Clock;
typedef struct OScInternal_Scanner OSc_Scanner;
typedef struct OScInternal_Detector OSc_Detector;
typedef struct OScInternal_Setting OSc_Setting;
typedef struct OScInternal_Acquisition OSc_Acquisition;

typedef void (*OSc_LogFunc)(const char *message, OSc_LogLevel level, void *data);

// Returns true normally, or false to halt the acquisition
typedef bool (*OSc_FrameCallback)(OSc_Acquisition *acq, uint32_t channel, void *pixels, void *data);

/** @} */ // addtogroup api

/**
 * \ingroup internal
 * \brief Check if OpenScanLib ABI version is compatible with the version used
 * to build the client application.
 */
bool OSc_API OScInternal_CheckVersion(uint32_t version);

/**
* \addtogroup api
* @{
*/

/**
 * \brief Check that a compatible version of OpenScanLib is being used.
 *
 * Determines if the version of OpenScanLib currently in use is compatible
 * with the version against which the application was built.
 *
 * \return `true` if the OpenScanLib is compatible; `false` otherwise
 */
static inline bool OSc_CheckVersion(void) {
	return OScInternal_CheckVersion(OScInternal_ABI_VERSION);
}

void OSc_API OSc_LogFunc_Set(OSc_LogFunc func, void *data);
void OSc_API OSc_Device_SetLogFunc(OSc_Device *device, OSc_LogFunc func, void *data);

void OSc_API OSc_SetDeviceModuleSearchPaths(char **paths);

OSc_Error OSc_API OSc_LSM_Create(OSc_LSM **lsm);
OSc_Error OSc_API OSc_LSM_Destroy(OSc_LSM *lsm);
OSc_Error OSc_API OSc_LSM_GetClock(OSc_LSM *lsm, OSc_Clock **clock);
OSc_Error OSc_API OSc_LSM_SetClock(OSc_LSM *lsm, OSc_Clock *clock);
OSc_Error OSc_API OSc_LSM_GetScanner(OSc_LSM *lsm, OSc_Scanner **scanner);
OSc_Error OSc_API OSc_LSM_SetScanner(OSc_LSM *lsm, OSc_Scanner *scanner);
OSc_Error OSc_API OSc_LSM_GetDetector(OSc_LSM *lsm, OSc_Detector **detector);
OSc_Error OSc_API OSc_LSM_SetDetector(OSc_LSM *lsm, OSc_Detector *detector);
OSc_Error OSc_API OSc_LSM_IsRunningAcquisition(OSc_LSM *lsm, bool *isRunning);

OSc_Error OSc_API OSc_GetAllDevices(OSc_Device ***devices, size_t *count);
OSc_Error OSc_API OSc_GetNumberOfAvailableDevices(size_t *count);

OSc_Error OSc_API OSc_Device_GetName(OSc_Device *device, const char **name);
OSc_Error OSc_API OSc_Device_GetDisplayName(OSc_Device *device, const char **name);
OSc_Error OSc_API OSc_Device_Open(OSc_Device *device, OSc_LSM *lsm);
OSc_Error OSc_API OSc_Device_Close(OSc_Device *device);
OSc_Error OSc_API OSc_Device_HasClock(OSc_Device *device, bool *hasClock);
OSc_Error OSc_API OSc_Device_HasScanner(OSc_Device *device, bool *hasScanner);
OSc_Error OSc_API OSc_Device_HasDetector(OSc_Device *device, bool *hasDetector);
OSc_Error OSc_API OSc_Device_GetClock(OSc_Device *device, OSc_Clock **clock);
OSc_Error OSc_API OSc_Device_GetScanner(OSc_Device *device, OSc_Scanner **scanner);
OSc_Error OSc_API OSc_Device_GetDetector(OSc_Device *device, OSc_Detector **detector);
OSc_Error OSc_API OSc_Device_GetSettings(OSc_Device *device, OSc_Setting ***settings, size_t *count);
OSc_Error OSc_API OSc_Device_GetAllowedResolutions(OSc_Device *device,
	size_t **widths, size_t **heights, size_t *count);
OSc_Error OSc_API OSc_Device_GetResolution(OSc_Device *device, size_t *width, size_t *height);
OSc_Error OSc_API OSc_Device_SetResolution(OSc_Device *device, size_t width, size_t height);
OSc_Error OSc_API OSc_Device_GetMagnification(OSc_Device *device, double *magnification);
OSc_Error OSc_API OSc_Device_SetMagnification(OSc_Device *device);

OSc_Error OSc_API OSc_Clock_GetDevice(OSc_Clock *clock, OSc_Device **device);

OSc_Error OSc_API OSc_Scanner_GetDevice(OSc_Scanner *scanner, OSc_Device **device);

OSc_Error OSc_API OSc_Detector_GetDevice(OSc_Detector *detector, OSc_Device **device);
OSc_Error OSc_API OSc_Detector_GetImageSize(OSc_Detector *detector, uint32_t *width, uint32_t *height);
OSc_Error OSc_API OSc_Detector_GetNumberOfChannels(OSc_Detector *detector, uint32_t *nChannels);
OSc_Error OSc_API OSc_Detector_GetBytesPerSample(OSc_Detector *detector, uint32_t *bytesPerSample);

OSc_Error OSc_API OSc_Setting_GetName(OSc_Setting *setting, char *name);
OSc_Error OSc_API OSc_Setting_GetValueType(OSc_Setting *setting, OSc_ValueType *valueType);
OSc_Error OSc_API OSc_Setting_IsEnabled(OSc_Setting *setting, bool *enabled);
OSc_Error OSc_API OSc_Setting_IsWritable(OSc_Setting *setting, bool *writable);
OSc_Error OSc_API OSc_Setting_GetNumericConstraintType(OSc_Setting *setting, OSc_ValueConstraint *constraintType);

OSc_Error OSc_API OSc_Setting_GetStringValue(OSc_Setting *setting, char *value);
OSc_Error OSc_API OSc_Setting_SetStringValue(OSc_Setting *setting, const char *value);

OSc_Error OSc_API OSc_Setting_GetBoolValue(OSc_Setting *setting, bool *value);
OSc_Error OSc_API OSc_Setting_SetBoolValue(OSc_Setting *setting, bool value);

OSc_Error OSc_API OSc_Setting_GetInt32Value(OSc_Setting *setting, int32_t *value);
OSc_Error OSc_API OSc_Setting_SetInt32Value(OSc_Setting *setting, int32_t value);
OSc_Error OSc_API OSc_Setting_GetInt32ContinuousRange(OSc_Setting *setting, int32_t *min, int32_t *max);
OSc_Error OSc_API OSc_Setting_GetInt32DiscreteValues(OSc_Setting *setting, int32_t **values, size_t *count);

OSc_Error OSc_API OSc_Setting_GetFloat64Value(OSc_Setting *setting, double *value);
OSc_Error OSc_API OSc_Setting_SetFloat64Value(OSc_Setting *setting, double value);
OSc_Error OSc_API OSc_Setting_GetFloat64ContinuousRange(OSc_Setting *setting, double *min, double *max);
OSc_Error OSc_API OSc_Setting_GetFloat64DiscreteValues(OSc_Setting *setting, double **values, size_t *count);

OSc_Error OSc_API OSc_Setting_GetEnumValue(OSc_Setting *setting, uint32_t *value);
OSc_Error OSc_API OSc_Setting_SetEnumValue(OSc_Setting *setting, uint32_t value);
OSc_Error OSc_API OSc_Setting_GetEnumNumValues(OSc_Setting *setting, uint32_t *count);
OSc_Error OSc_API OSc_Setting_GetEnumNameForValue(OSc_Setting *setting, uint32_t value, char *name);
OSc_Error OSc_API OSc_Setting_GetEnumValueForName(OSc_Setting *setting, uint32_t *value, const char *name);

OSc_Error OSc_API OSc_Acquisition_Create(OSc_Acquisition **acq, OSc_LSM *lsm);
OSc_Error OSc_API OSc_Acquisition_Destroy(OSc_Acquisition *acq);
OSc_Error OSc_API OSc_Acquisition_SetNumberOfFrames(OSc_Acquisition *acq, uint32_t numberOfFrames);
OSc_Error OSc_API OSc_Acquisition_SetFrameCallback(OSc_Acquisition *acq, OSc_FrameCallback callback);
OSc_Error OSc_API OSc_Acquisition_GetData(OSc_Acquisition *acq, void **data);
OSc_Error OSc_API OSc_Acquisition_SetData(OSc_Acquisition *acq, void *data);
OSc_Error OSc_API OSc_Acquisition_Arm(OSc_Acquisition *acq);
OSc_Error OSc_API OSc_Acquisition_Start(OSc_Acquisition *acq);
OSc_Error OSc_API OSc_Acquisition_Stop(OSc_Acquisition *acq);
OSc_Error OSc_API OSc_Acquisition_Wait(OSc_Acquisition *acq);

/** @} */ // addtogroup api

#ifdef __cplusplus
} // extern "C"
#endif
