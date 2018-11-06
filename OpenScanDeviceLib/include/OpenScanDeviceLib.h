/** \file
 * \brief Public header for OpenScanDeviceLib.
 *
 * This header is to be included by modules implementing OpenScan devices.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


/** \mainpage OpenScanDeviceLib: OpenScan Device Programming Interface
 *
 * OpenScanLib uses loadable modules (Windows DLLs, macOS bundles, Linux
 * shared objects) to control each device type. These are known as OpenScan
 * device modules.
 *
 * OpenScanDeviceLib defines the interface to which OpenScan device modules
 * are programmed. It is built as a static library and must be linked by every
 * device module.
 *
 * For documentation on writing device modules, see \ref dpi.
 *
 * For documentation on the internals of OpenScanDeviceLib, see \ref internal.
 */

/**
 * \defgroup dpi OpenScan Device Programming Interface
 *
 * OpenScanDeviceLib is a static library to which every OpenScan device module
 * must link, and contains the mechanism that allows devices to call functions
 * in OpenScanLib, as well as the mechanism by which OpenScanLib accesses the
 * device module.
 *
 * To create a new device module, use the macro #OScDev_MODULE_IMPL to provide
 * all required data and implementation functions.
 */

/**
 * \defgroup internal OpenScanDeviceLib Internals
 *
 * These are the internal workings of OpenScanDeviceLib; device modules must
 * not used these declarations and definitions directly.
 */


/// Create an ABI version number
/** \ingroup internal
 * \see OScDevInternal_ABI_VERSION
 */
#define OScDevInternal_MAKE_VERSION(major, minor) ((uint16_t)(major << 16) | (uint16_t)minor)


/// Binary interface version between device module and OpenScanLib.
/** \ingroup internal
 * Device modules should not need to be concerned with this version number.
 * It is used to allow OpenScanLib to correctly interact with (or reject)
 * device modules built with different versions of OpenScanDeviceLib.
 *
 * Since this version number applies to the binary interface, it has no
 * particular relationship to the release version number of OpenScanLib or
 * OpenScanDeviceLib.
 *
 * The version number is a 32-bit unsigned integer, with the high 16 bits
 * representing the major version and the low 16 bits the minor version.
 *
 * The minor version must be incremented when one of the following changes
 * are made:
 * - A module-to-OpenScanLib call is added at the end of
 *   `OScDevInternal_Interface`
 * - A previously required device implementation field becomes optional
 * - An optional field is added at the end of a device implementation struct
 * - A new `enum` constant is added (at the end) to an `enum` type that is
 *   not interpreted by modules
 *
 * The major version must be incremented (and the minor version reset to 0)
 * when any other change is made.
 */
#define OScDevInternal_ABI_VERSION OScDevInternal_MAKE_VERSION(0, 0)


/** \addtogroup dpi
 * @{
 */


// Forward declarations for types defined later in this header
struct OScDev_ModuleImpl;
struct OScDev_DeviceImpl;
struct OScDev_SettingImpl;


/// Maximum length for strings copied to provided buffer.
#define OScDev_MAX_STR_LEN 511


// Declarations for opaque data types accessed through device interface
// functions.
typedef struct OSc_Device OScDev_Device;
typedef struct OSc_Setting OScDev_Setting;
typedef struct OSc_Acquisition OScDev_Acquisition;


/// Log level.
enum OScDev_LogLevel
{
	OScDev_LogLevel_Debug,
	OScDev_LogLevel_Info,
	OScDev_LogLevel_Warning,
	OScDev_LogLevel_Error,
};


/// Error return value.
/**
 * Zero (`OScDev_OK`) indicates success.
 *
 * This will be replaced with an opaque type in the future.
 */
typedef int32_t OScDev_Error;
enum { OScDev_OK = 0 };


/// Convenience macro for checking error return values.
/**
 * For example,
 *
 *     OScDev_Error err;
 *     if (OScDev_CHECK(err, OScDev_Device_Create(...)))
 *     {
 *         // clean up
 *         return err;
 *     }
 */
#define OScDev_CHECK(err, call) ((err = (call)) != OScDev_OK)

// Note: There is no OScDev_RETURN_IF_ERROR macro because it tends to
// encourage poor coding practices. Often, locally allocated resources need to
// be freed before returning with an error.


enum OScDev_ValueType
{
	OScDev_ValueType_String,
	OScDev_ValueType_Bool,
	OScDev_ValueType_Int32,
	OScDev_ValueType_Float64,
	OScDev_ValueType_Enum,
};


enum OScDev_ValueConstraint
{
	OScDev_ValueConstraint_None,
	OScDev_ValueConstraint_DiscreteValues,
	OScDev_ValueConstraint_Range,
};


/** @} */ // addtogroup dpi


/// Pointer to the interface function table provided by OpenScanLib.
/** \ingroup internal
 */
extern struct OScDevInternal_Interface *OScDevInternal_FunctionTable;


/// Pointer to the module implementation provided by the device module.
/** \ingroup internal
 * \see OScDev_DEFINE_MODULE_IMPL
 */
extern struct OScDev_ModuleImpl OScDevInternal_TheModuleImpl;


/// Define the device module implementation.
/** \ingroup dpi
 * Each device module must use this macro exactly once, in an implementation
 * file (i.e. not in a header), to define the module implementation.
 *
 * For example,
 *
 *     OScDev_MODULE_IMPL =
 *     {
 *         .displayName = "MyDevice",
 *         .Open = MyDeviceOpen,
 *         .Close = MyDeviceClose,
 *         // ...
 *     };
 *
 * Within the braces, all required fields (and any desired optional fields) of
 * #OScDev_ModuleImpl must be initialized.
 *
 * \see OScDev_ModuleImpl
 */
#define OScDev_MODULE_IMPL struct OScDev_ModuleImpl OScDevInternal_TheModuleImpl


/// Interface function table for module to call OpenScanLib.
/** \ingroup internal
 * The fields and layout of this struct must not be altered without a
 * corresponding increment of the device interface version.
 */
struct OScDevInternal_Interface
{
	void (*Log)(struct OScDev_ModuleImpl *modImpl, OScDev_Device *device, enum OScDev_LogLevel level, const char *message);

	OScDev_Error (*Device_Create)(struct OScDev_ModuleImpl *modImpl, OScDev_Device **device, struct OScDev_DeviceImpl *impl, void *data);
	void *(*Device_GetImplData)(struct OScDev_ModuleImpl *modImpl, OScDev_Device *device);

	OScDev_Error (*Setting_Create)(struct OScDev_ModuleImpl *modImpl, OScDev_Setting **setting, OScDev_Device *device, const char *name, enum OScDev_ValueType valueType, struct OScDev_SettingImpl *impl, void *data);
	OScDev_Device *(*Setting_GetDevice)(struct OScDev_ModuleImpl *modImpl, OScDev_Setting *setting);
	void *(*Setting_GetImplData)(struct OScDev_ModuleImpl *modImpl, OScDev_Setting *setting);

	OScDev_Error (*Acquisition_GetNumberOfFrames)(struct OScDev_ModuleImpl *modImpl, OScDev_Acquisition *acq, uint32_t *numberOfFrames);
	bool (*Acquisition_CallFrameCallback)(struct OScDev_ModuleImpl *modImpl, OScDev_Acquisition *acq, uint32_t channel, void *pixels);
};


/// The module implementation function table.
/** \ingroup dpi
 *
 * Modules define an instance of this struct using the macro
 * #OScDev_MODULE_IMPL.
 *
 * Fields marked **Optional** can be left `NULL`. Required fields must be set.
 *
 * \see OScDev_MODULE_IMPL
 */
struct OScDev_ModuleImpl
{
	/// Human-readable name of this module.
	/**
	 * Implementations should set this to a string constant.
	 *
	 * Note that this should be a human-readable name (e.g. "OpenScan NIDAQ").
	 * The module name for programmatic access is taken from the module's
	 * filename and is not affected by this string.
	 *
	 * **Required**, must not be `NULL`.
	 */
	const char *displayName;

	/// Called before OpenScanLib accesses any other functions in this module.
	/**
	 * This can be used, for example, to dynamically load a device SDK library
	 * or to initialize an SDK library.
	 *
	 * **Optional**
	 */
	OScDev_Error (*Open)(void);

	/// Called after OpenScanLib has finished accessing this module.
	/**
	 * This can be used, for example, to uninitialize a device SDK library if
	 * required.
	 *
	 * **Optional**
	 */
	OScDev_Error (*Close)(void);

	// TODO We need to use DeviceLoader(?) objects to handle flexible device
	// enumeration. For now, we temporarily continue to directly enumerate
	// (pre-created) devices.
	/// Enumerate and create all devices.
	/**
	 * Note: The device enumeration mechanism will likely change in the near
	 * future.
	 *
	 * **Required**
	 */
	OScDev_Error (*GetDeviceImpls)(struct OScDev_DeviceImpl **impls, size_t *implCount);
};


struct OScDev_DeviceImpl
{
	OScDev_Error (*GetModelName)(const char **name);
	OScDev_Error (*GetInstances)(OScDev_Device ***devices, size_t *count);
	OScDev_Error (*ReleaseInstance)(OScDev_Device *device);

	OScDev_Error (*GetName)(OScDev_Device *device, char *name);
	OScDev_Error (*Open)(OScDev_Device *device);
	OScDev_Error (*Close)(OScDev_Device *device);

	OScDev_Error (*HasScanner)(OScDev_Device *device, bool *hasScanner);
	OScDev_Error (*HasDetector)(OScDev_Device *device, bool *hasDetector);

	OScDev_Error (*GetSettings)(OScDev_Device *device, OScDev_Setting ***settings, size_t *count);

	OScDev_Error (*GetAllowedResolutions)(OScDev_Device *device, size_t **widths, size_t **heights, size_t *count);
	OScDev_Error (*GetResolution)(OScDev_Device *device, size_t *width, size_t *height);
	OScDev_Error (*SetResolution)(OScDev_Device *device, size_t width, size_t height);

	OScDev_Error (*GetMagnification)(OScDev_Device *device, double *magnification);
	OScDev_Error (*SetMagnification)(OScDev_Device *device);

	OScDev_Error (*GetImageSize)(OScDev_Device *device, uint32_t *width, uint32_t *height);
	OScDev_Error (*GetNumberOfChannels)(OScDev_Device *device, uint32_t *nChannels);
	OScDev_Error (*GetBytesPerSample)(OScDev_Device *device, uint32_t *bytesPerSample);

	OScDev_Error (*ArmScanner)(OScDev_Device *device, OScDev_Acquisition *acq);
	OScDev_Error (*StartScanner)(OScDev_Device *device, OScDev_Acquisition *acq);
	OScDev_Error (*StopScanner)(OScDev_Device *device, OScDev_Acquisition *acq);

	OScDev_Error (*ArmDetector)(OScDev_Device *device, OScDev_Acquisition *acq);
	OScDev_Error (*StartDetector)(OScDev_Device *device, OScDev_Acquisition *acq);
	OScDev_Error (*StopDetector)(OScDev_Device *device, OScDev_Acquisition *acq);

	OScDev_Error (*IsRunning)(OScDev_Device *device, bool *isRunning);
	OScDev_Error (*Wait)(OScDev_Device *device);
};


struct OScDev_SettingImpl
{
	OScDev_Error (*IsEnabled)(OScDev_Setting *setting, bool *enabled);
	OScDev_Error (*IsWritable)(OScDev_Setting *setting, bool *writable);
	OScDev_Error (*GetNumericConstraintType)(OScDev_Setting *setting, enum OScDev_ValueConstraint *constraintType);
	OScDev_Error (*GetString)(OScDev_Setting *setting, char *value);
	OScDev_Error (*SetString)(OScDev_Setting *setting, const char *value);
	OScDev_Error (*GetBool)(OScDev_Setting *setting, bool *value);
	OScDev_Error (*SetBool)(OScDev_Setting *setting, bool value);
	OScDev_Error (*GetInt32)(OScDev_Setting *setting, int32_t *value);
	OScDev_Error (*SetInt32)(OScDev_Setting *setting, int32_t value);
	OScDev_Error (*GetInt32Range)(OScDev_Setting *setting, int32_t *min, int32_t *max);
	OScDev_Error (*GetInt32DiscreteValues)(OScDev_Setting *setting, int32_t **values, size_t *count);
	OScDev_Error (*GetFloat64)(OScDev_Setting *setting, double *value);
	OScDev_Error (*SetFloat64)(OScDev_Setting *setting, double value);
	OScDev_Error (*GetFloat64Range)(OScDev_Setting *setting, double *min, double *max);
	OScDev_Error (*GetFloat64DiscreteValues)(OScDev_Setting *setting, double **values, size_t *count);
	OScDev_Error (*GetEnum)(OScDev_Setting *setting, uint32_t *value);
	OScDev_Error (*SetEnum)(OScDev_Setting *setting, uint32_t value);
	OScDev_Error (*GetEnumNumValues)(OScDev_Setting *setting, uint32_t *count);
	OScDev_Error (*GetEnumNameForValue)(OScDev_Setting *setting, uint32_t value, char *name);
	OScDev_Error (*GetEnumValueForName)(OScDev_Setting *setting, uint32_t *value, const char *name);
};


#ifdef __cplusplus
} // extern "C"
#endif


//
// Inline functions wrapping all entries in the OpenScanLib function table
//

#ifdef __cplusplus
/// Inline keyword for C++
#define OScDevInternal_INLINE inline
#else // C
/// Inline keyword for C
#define OScDevInternal_INLINE static inline
#endif

/** \addtogroup dpi
 * @{
 */

/// Log a message
OScDevInternal_INLINE void OScDev_Log(OScDev_Device *device, enum OScDev_LogLevel level, const char *message)
{
	OScDevInternal_FunctionTable->Log(&OScDevInternal_TheModuleImpl, device, level, message);
}

/// Log a debug-level message
OScDevInternal_INLINE void OScDev_LogDebug(OScDev_Device *device, const char *message)
{
	OScDev_Log(device, OScDev_LogLevel_Debug, message);
}

/// Log an info-level message
OScDevInternal_INLINE void OScDev_LogInfo(OScDev_Device *device, const char *message)
{
	OScDev_Log(device, OScDev_LogLevel_Info, message);
}

/// Log a warning-level message
OScDevInternal_INLINE void OScDev_LogWarning(OScDev_Device *device, const char *message)
{
	OScDev_Log(device, OScDev_LogLevel_Warning, message);
}

/// Log an error-level message
OScDevInternal_INLINE void OScDev_LogError(OScDev_Device *device, const char *message)
{
	OScDev_Log(device, OScDev_LogLevel_Error, message);
}

OScDevInternal_INLINE OScDev_Error OScDev_Device_Create(OScDev_Device **device, struct OScDev_DeviceImpl *impl, void *data)
{
	return OScDevInternal_FunctionTable->Device_Create(&OScDevInternal_TheModuleImpl, device, impl, data);
}

OScDevInternal_INLINE void *OScDev_Device_GetImplData(OScDev_Device *device)
{
	return OScDevInternal_FunctionTable->Device_GetImplData(&OScDevInternal_TheModuleImpl, device);
}

OScDevInternal_INLINE OScDev_Error OScDev_Setting_Create(OScDev_Setting **setting, OScDev_Device *device, const char *name, enum OScDev_ValueType valueType, struct OScDev_SettingImpl *impl, void *data)
{
	return OScDevInternal_FunctionTable->Setting_Create(&OScDevInternal_TheModuleImpl, setting, device, name, valueType, impl, data);
}

OScDevInternal_INLINE OScDev_Device *OScDev_Setting_GetDevice(OScDev_Setting *setting)
{
	return OScDevInternal_FunctionTable->Setting_GetDevice(&OScDevInternal_TheModuleImpl, setting);
}

OScDevInternal_INLINE void *OScDev_Setting_GetImplData(OScDev_Setting *setting)
{
	return OScDevInternal_FunctionTable->Setting_GetImplData(&OScDevInternal_TheModuleImpl, setting);
}

OScDevInternal_INLINE OScDev_Error OScDev_Acquisition_GetNumberOfFrames(OScDev_Acquisition *acq, uint32_t *numberOfFrames)
{
	return OScDevInternal_FunctionTable->Acquisition_GetNumberOfFrames(&OScDevInternal_TheModuleImpl, acq, numberOfFrames);
}

OScDevInternal_INLINE bool OScDev_Acquisition_CallFrameCallback(OScDev_Acquisition *acq, uint32_t channel, void *pixels)
{
	return OScDevInternal_FunctionTable->Acquisition_CallFrameCallback(&OScDevInternal_TheModuleImpl, acq, channel, pixels);
}


/** @} */ // addtogroup dpi
