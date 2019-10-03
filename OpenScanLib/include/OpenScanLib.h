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
#define OScInternal_ABI_VERSION OScInternal_MAKE_VERSION(2, 0)

/**
 * \addtogroup api
 * @{
 */

/**
 * \brief Buffer size for fixed-length strings.
 */
#define OSc_MAX_STR_SIZE 512

/**
 * \brief Maximum length for fixed-length string buffers.
 *
 * Note that the buffer size must be #OSc_MAX_STR_SIZE.
 */
#define OSc_MAX_STR_LEN (OSc_MAX_STR_SIZE - 1)


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


/**
 * \brief An LSM object, which integrates clock, scanner, and detector
 * functionality.
 */
typedef struct OScInternal_LSM OSc_LSM;

/**
 * \brief A device object, which models a particular hardware device.
 *
 * A device has some combination of clock, scanner, and/or detector capability.
 */
typedef struct OScInternal_Device OSc_Device;

/**
 * \brief A setting object, representing a device parameter.
 */
typedef struct OScInternal_Setting OSc_Setting;

/**
 * \brief An acquisition object, managing the state and data transfer for
 * data acquisition.
 */
typedef struct OScInternal_Acquisition OSc_Acquisition;

/**
 * \brief Pointer to a logger function.
 * \sa OSc_LogFunc_Set()
 * \sa OSc_Device_SetLogFunc()
 */
typedef void (*OSc_LogFunc)(const char *message, OSc_LogLevel level, void *data);

/**
 * \brief Pointer to function that receives acquired frame data.
 * \sa OSc_Acquisition_SetFrameCallback()
 * \param acq the acquisition
 * \param channel the channel number (zero-based)
 * \param pixels image data for the given channel, which the frame callback must copy
 * \param data the acquisition client data set by OSc_Acquisition_SetData()
 * \return `true` normally, or `false` to cancel the acquisition
 */
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

/**
 * \brief Set the logger for OpenScan.
 *
 * The given function will be used for all logging by OpenScanLib and device
 * modules, unless a device-specific logger is used.
 *
 * \param func the logger function, or null, in which case the logger is removed
 * \param data client data passed to the logger function
 * \sa OSc_Device_SetLogFunc()
 */
void OSc_API OSc_LogFunc_Set(OSc_LogFunc func, void *data);

/**
 * \brief Set the logger for a particular device.
 *
 * If \p device is null, nothing is done.
 *
 * \param device the device for which the logger should be used
 * \param func the logger function, or null, in which case the logger is removed
 * \param data client data passed to the logger function
 * \sa OSc_LogFunc_Set()
 */
void OSc_API OSc_Device_SetLogFunc(OSc_Device *device, OSc_LogFunc func, void *data);

/**
 * \brief Set the search path for device modules.
 *
 * This function must be called before either OSc_GetAllDevices() or
 * OSc_GetNumberOfAvailableDevices() is called.
 *
 * OpenScanLib makes a copy of the path strings, so they can be released after
 * the call.
 *
 * \param paths pointer to array of pointers to path strings
 */
void OSc_API OSc_SetDeviceModuleSearchPaths(char **paths);

OSc_Error OSc_API OSc_LSM_Create(OSc_LSM **lsm);

/**
 * \todo Return value should be `void`.
 */
OSc_Error OSc_API OSc_LSM_Destroy(OSc_LSM *lsm);

OSc_API OSc_Device *OSc_LSM_GetClockDevice(OSc_LSM *lsm);

OSc_API OSc_Device *OSc_LSM_GetScannerDevice(OSc_LSM *lsm);

OSc_API OSc_Device *OSc_LSM_GetDetectorDevice(OSc_LSM *lsm);

OSc_Error OSc_API OSc_LSM_SetClockDevice(OSc_LSM *lsm, OSc_Device *clockDevice);

OSc_Error OSc_API OSc_LSM_SetScannerDevice(OSc_LSM *lsm, OSc_Device *scannerDevice);

OSc_Error OSc_API OSc_LSM_SetDetectorDevice(OSc_LSM *lsm, OSc_Device *detectorDevice);

/**
 * \todo This function should be retired once we revise the lifecycle of
 * acquisition objects to properly separate arm-disarm from start-stop.
 */
OSc_Error OSc_API OSc_LSM_IsRunningAcquisition(OSc_LSM *lsm, bool *isRunning);

/**
 * \brief Get all available device instances.
 *
 * The returned array of devices remains valid indefinitely.
 *
 * \param[out] devices location where pointer to array of pointers to devices will be written
 * \param[out] count location where number of devices in the array will be written
 *
 * \todo Return value should be `void` as we skip any modules and device
 * implementations that have an error. Or we could also return an array of
 * errors.
 *
 * \todo This API precludes re-enumerating devices. Now that device objects
 * are newly created every time enumeration is performed, we should use a new
 * interface: `OSc_EnumerateDevices(OSc_PtrArray *devices)`.
 *
 * \todo We will also need support for non-enumerable devices once we add
 * support for them in the device interface.
 */
OSc_Error OSc_API OSc_GetAllDevices(OSc_Device ***devices, size_t *count);

/**
 * \deprecated Redundant.
 */
OSc_Error OSc_API OSc_GetNumberOfAvailableDevices(size_t *count);

/**
 * \brief Get the name of a device.
 *
 * \param device the device
 * \param[out] name location where pointer to name string will be written
 *
 * \todo This API requires OpenScanLib to keep the name string available
 * indefinitely. We should copy to client location.
 *
 * \todo Return value should be `void`; we can return "" if device is null
 * and do nothing if destination is null.
 */
OSc_Error OSc_API OSc_Device_GetName(OSc_Device *device, const char **name);

/**
 * \brief Get the name of a device in a format including its model name.
 *
 * \param device the device
 * \param[out] name location where pointer to name string will be written
 *
 * \todo Replace this with `OSc_Device_GetModelName()` and possibly
 * `OSc_Device_GetModuleName()`. Formatting the name as `"name@model"` should
 * be the applications responsibility. Same with ensuring that names are
 * unique.
 */
OSc_Error OSc_API OSc_Device_GetDisplayName(OSc_Device *device, const char **name);

OSc_Error OSc_API OSc_Device_Open(OSc_Device *device, OSc_LSM *lsm);

/**
 * \todo Return value should be `void`.
 */
OSc_Error OSc_API OSc_Device_Close(OSc_Device *device);

OSc_Error OSc_API OSc_Device_HasClock(OSc_Device *device, bool *hasClock);
OSc_Error OSc_API OSc_Device_HasScanner(OSc_Device *device, bool *hasScanner);
OSc_Error OSc_API OSc_Device_HasDetector(OSc_Device *device, bool *hasDetector);

/**
 * \brief Get the settings for a device.
 *
 * The device must be open. The array of settings returned by this function
 * remain valid until the device is closed.
 *
 * \param device the device
 * \param[out] settings location where pointer to array of pointers to settings will be written
 * \param[out] count location where number of settings in the array will be written
 *
 * \todo While it makes sense that settings remain valid until the device is
 * closed, the array in which they are returned should be owned by the caller.
 */
OSc_Error OSc_API OSc_Device_GetSettings(OSc_Device *device, OSc_Setting ***settings, size_t *count);

/**
 * \brief Get the scan resolutions supported by a device.
 *
 * \param device the device
 * \param[out] widths location where pointer to array of width values will be written
 * \param[out] heights location where pointer to array of height values will be written
 * \param[out] count location where number of resolutions in the two arrays will be written
 *
 * \todo Resolution should be a single number.
 *
 * \todo Resolution should be set for an acquisition, not individual devices.
 * The job of finding resolutions that are valid for all participating devices
 * should be OpenScanLib's responsibility.
 */
OSc_Error OSc_API OSc_Device_GetAllowedResolutions(OSc_Device *device,
	size_t **widths, size_t **heights, size_t *count);

/**
 * \todo Resolution should be a parameter of an acquisition, not devices.
 */
OSc_Error OSc_API OSc_Device_GetResolution(OSc_Device *device, size_t *width, size_t *height);

/**
* \todo Resolution should be a parameter of an acquisition, not devices.
*/
OSc_Error OSc_API OSc_Device_SetResolution(OSc_Device *device, size_t width, size_t height);

/**
 * \todo Magnification should be managed by the application; zoom factor should
 * be a parameter of an acquisition.
 */
OSc_Error OSc_API OSc_Device_GetMagnification(OSc_Device *device, double *magnification);

/**
* \todo Magnification should be managed by the application; zoom factor should
* be a parameter of an acquisition.
*/
OSc_Error OSc_API OSc_Device_SetMagnification(OSc_Device *device);

/**
 * \todo Image size should be determined by the acquisition (or acquisition
 * template) based on parameters.
 */
OSc_Error OSc_API OSc_Device_GetDetectorImageSize(OSc_Device *detectorDevice, uint32_t *width, uint32_t *height);

/**
 * \todo Once we have "acquisition settings", number of channels should be a
 * property of acquisition (or acquisition template), not detector device.
 */
OSc_Error OSc_API OSc_Device_GetDetectorNumberOfChannels(OSc_Device *detectorDevice, uint32_t *nChannels);

/**
 * \todo Sample format should be a property of acquisition (or acquisition
 * template), not detector device.
 */
OSc_Error OSc_API OSc_Device_GetDetectorBytesPerSample(OSc_Device *detectorDevice, uint32_t *bytesPerSample);

/**
 * \brief Get the name of a setting.
 *
 * \param setting the setting.
 * \param[out] name a string buffer of size at least #OSc_MAX_STR_SIZE, where the name will be written.
 */
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
