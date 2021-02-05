/** \file
 * \brief Public header for OpenScanDeviceLib.
 *
 * This header is to be included by modules implementing OpenScan devices.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

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
 * not use these declarations and definitions directly.
 */


/// Create an ABI version number
/** \ingroup internal
 * \see OScDevInternal_ABI_VERSION
 */
#define OScDevInternal_MAKE_VERSION(major, minor) (((uint32_t)(major) << 16) | (uint16_t)(minor))


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
 * Device modules written for version `x.y` will continue to work with a
 * version of OpenScanLib with device ABI version `x.z` where `z >= y`,
 * without any special handling. However, it will not work with an
 * OpenScanLib with device ABI version `x.w` where `w < y`. This intent
 * leads to the following policy (which may not cover all edge cases):
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
 *
 * Version numbers should be set correctly for each Git commit. If a related
 * set of changes is to be made over multiple commits, the version number
 * can be set to `(-1, 0)` in intermediate commits to indicate "experimental".
 */
#define OScDevInternal_ABI_VERSION OScDevInternal_MAKE_VERSION(12, 0)


/** \addtogroup dpi
 * @{
 */


// Forward declarations for types defined later in this header
typedef struct OScDev_ModuleImpl OScDev_ModuleImpl;
typedef struct OScDev_DeviceImpl OScDev_DeviceImpl;
typedef struct OScDev_SettingImpl OScDev_SettingImpl;

/// Buffer size for fixed-length strings.
#define OScDev_MAX_STR_SIZE 512

/// Maximum length for strings copied to provided buffer.
#define OScDev_MAX_STR_LEN (OScDev_MAX_STR_SIZE - 1)


// Declarations for opaque data types accessed through device interface
// functions.
typedef struct OScInternal_Device OScDev_Device;
typedef struct OScInternal_Setting OScDev_Setting;
typedef struct OScInternal_AcquisitionForDevice OScDev_Acquisition;
typedef struct RERR_Error OScDev_RichError;


/// Log level.
typedef int32_t OScDev_LogLevel;

/// Constants for ::OScDev_LogLevel.
enum
{
	OScDev_LogLevel_Debug,
	OScDev_LogLevel_Info,
	OScDev_LogLevel_Warning,
	OScDev_LogLevel_Error,
};

typedef int32_t OScDev_ErrorCodeFormat;
enum
{

};


/// Error return value.
/**
 * Zero (`OScDev_OK`) indicates success.
 *
 * This will be replaced with an opaque type in the future.
 */
typedef int32_t OScDev_Error;
enum {
	OScDev_OK = 0,

	// For now, we define a number of standard error codes here. Error handling
	// will be upgraded soon to use rich error reporting.

	// WARNING: Do not edit these without correctly incrementing the device
	// interface version.
	// WARNING 2: These must exactly match the OSc_Error_* constants defined
	// in OpenScanLib.h.
	OScDev_Error_Unknown = 10000,
	OScDev_Error_Unsupported_Operation,
	OScDev_Error_Illegal_Argument,
	OScDev_Error_Device_Module_Already_Exists,
	OScDev_Error_No_Such_Device_Module,
	OScDev_Error_Driver_Not_Available,
	OScDev_Error_Device_Already_Open,
	OScDev_Error_Device_Not_Opened_For_LSM,
	OScDev_Error_Device_Does_Not_Support_Clock,
	OScDev_Error_Device_Does_Not_Support_Scanner,
	OScDev_Error_Device_Does_Not_Support_Detector,
	OScDev_Error_Wrong_Value_Type,
	OScDev_Error_Setting_Not_Writable,
	OScDev_Error_Wrong_Constraint_Type,
	OScDev_Error_Unknown_Enum_Value_Name,
	OScDev_Error_Acquisition_Running,
	OScDev_Error_Not_Armed,
	OScDev_Error_Waveform_Out_Of_Range,
	OScDev_Error_Waveform_Memory_Size_Mismatch,
	OScDev_Error_Data_Left_In_Fifo_After_Reading_Image,
};


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


typedef int32_t OScDev_TriggerSource;
enum
{
	OScDev_TriggerSource_Software,
	OScDev_TriggerSource_External,
};


typedef int32_t OScDev_ClockSource;
enum
{
	OScDev_ClockSource_Internal,
	OScDev_ClockSource_External,
};


typedef int32_t OScDev_ValueType;
enum
{
	OScDev_ValueType_String,
	OScDev_ValueType_Bool,
	OScDev_ValueType_Int32,
	OScDev_ValueType_Float64,
	OScDev_ValueType_Enum,
};


typedef int32_t OScDev_ValueConstraint;
enum
{
	OScDev_ValueConstraint_None,
	OScDev_ValueConstraint_DiscreteValues,
	OScDev_ValueConstraint_Range,
};


/// Dynamic or static array of pointers.
/**
 * This data type is used to pass lists of objects from device modules to
 * OpenScanLib.
 *
 * Whether the pointers contained in the array are "owned" (in the
 * resource-management sense) by the array depends on the usage context; make
 * sure to check the documentation of the function accepting or returning the
 * array.
 *
 * An array can be created dynamically by calling OScDev_PtrArray_Create().
 *
 * (Because this type is intended solely for passing short lists, the only
 * available operation is appending elements.)
 */
typedef struct OScInternal_PtrArray OScDev_PtrArray;


/// Dynamic or static array of numbers.
/**
 * This data type is used to pass lists of numbers from device modules to
 * OpenScanLib.
 *
 * Although it holds double values, we use it for integer lists, too, for
 * simplicity.
 *
 * An array can be created dynamically by calling OScDev_NumArray_Create().
 *
 * (Because this type is intended solely for passing short lists, the only
 * available operation is appending elements.)
 */
typedef struct OScInternal_NumArray OScDev_NumArray;


/// Continuous or discrete numerical range.
/**
 * This data type is used to mass numerical ranges from device modules to
 * OpenScanLib.
 *
 * A range can be created dynamically by calling
 * OScDev_NumRange_CreateContinuous() or OScDev_NumRange_CreateDiscrete().
 */
typedef struct OScInternal_NumRange OScDev_NumRange;


/** @} */ // addtogroup dpi

#ifndef OScDevInternal_BUILDING_OPENSCANLIB

/// Pointer to the interface function table provided by OpenScanLib.
/** \ingroup internal
 */
extern struct OScDevInternal_Interface *OScDevInternal_FunctionTable;


/// Pointer to the module implementation provided by the device module.
/** \ingroup internal
 * \see OScDev_DEFINE_MODULE_IMPL
 */
extern OScDev_ModuleImpl OScDevInternal_TheModuleImpl;

#endif


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
#define OScDev_MODULE_IMPL OScDev_ModuleImpl OScDevInternal_TheModuleImpl


/// Interface function table for module to call OpenScanLib.
/** \ingroup internal
 * The fields and layout of this struct must not be altered without a
 * corresponding increment of the device interface version.
 */
struct OScDevInternal_Interface
{
	// All functions take a pointer to the module implementation, even if not
	// needed in the implementation, because it may help with debugging.

	void (*Log)(OScDev_ModuleImpl *modImpl, OScDev_Device *device, OScDev_LogLevel level, const char *message);

	OScDev_RichError *(*Error_RegisterCodeDomain)(OScDev_ModuleImpl* modImpl, const char* domainName, OScDev_ErrorCodeFormat codeFormat);
	OScDev_Error (*Error_ReturnAsCode)(OScDev_ModuleImpl* modImpl, OScDev_RichError *error);
	OScDev_RichError *(*Error_Create)(OScDev_ModuleImpl* modImpl, const char* domainName, OScDev_Error code, const char* message);

	OScDev_PtrArray *(*PtrArray_Create)(OScDev_ModuleImpl *modImpl);
	OScDev_PtrArray *(*PtrArray_CreateFromNullTerminated)(OScDev_ModuleImpl *modImpl, void *const *nullTerminatedArray);
	void (*PtrArray_Destroy)(OScDev_ModuleImpl *modImpl, const OScDev_PtrArray *arr);
	void (*PtrArray_Append)(OScDev_ModuleImpl *modImpl, OScDev_PtrArray *arr, void *obj);
	size_t (*PtrArray_Size)(OScDev_ModuleImpl *modImpl, const OScDev_PtrArray *arr);
	bool (*PtrArray_Empty)(OScDev_ModuleImpl *modImpl, const OScDev_PtrArray *arr);
	void *(*PtrArray_At)(OScDev_ModuleImpl *modImpl, const OScDev_PtrArray *arr, size_t index);

	OScDev_NumArray *(*NumArray_Create)(OScDev_ModuleImpl *modImpl);
	OScDev_NumArray *(*NumArray_CreateFromNaNTerminated)(OScDev_ModuleImpl *modImpl, const double *nanTerminatedArray);
	void (*NumArray_Destroy)(OScDev_ModuleImpl *modImpl, const OScDev_NumArray *arr);
	void (*NumArray_Append)(OScDev_ModuleImpl *modImpl, OScDev_NumArray *arr, double val);
	size_t (*NumArray_Size)(OScDev_ModuleImpl *modImpl, const OScDev_NumArray *arr);
	bool (*NumArray_Empty)(OScDev_ModuleImpl *modImpl, const OScDev_NumArray *arr);
	double (*NumArray_At)(OScDev_ModuleImpl *modImpl, const OScDev_NumArray *arr, size_t index);

	OScDev_NumRange *(*NumRange_CreateContinuous)(OScDev_ModuleImpl *modImpl, double rMin, double rMax);
	OScDev_NumRange *(*NumRange_CreateDiscrete)(OScDev_ModuleImpl *modImpl);
	OScDev_NumRange *(*NumRange_CreateDiscreteFromNaNTerminated)(OScDev_ModuleImpl *modImpl, const double *nanTerminatedArray);
	void (*NumRange_Destroy)(OScDev_ModuleImpl *modImpl, const OScDev_NumRange *range);
	void (*NumRange_AppendDiscrete)(OScDev_ModuleImpl *modImpl, OScDev_NumRange *range, double val);

	OScDev_Error (*Device_Create)(OScDev_ModuleImpl *modImpl, OScDev_Device **device, OScDev_DeviceImpl *impl, void *data);
	void *(*Device_GetImplData)(OScDev_ModuleImpl *modImpl, OScDev_Device *device);

	OScDev_Error (*Setting_Create)(OScDev_ModuleImpl *modImpl, OScDev_Setting **setting, const char *name, OScDev_ValueType valueType, OScDev_SettingImpl *impl, void *data);
	void (*Setting_Destroy)(OScDev_ModuleImpl *modImpl, OScDev_Setting *setting);
	void *(*Setting_GetImplData)(OScDev_ModuleImpl *modImpl, OScDev_Setting *setting);

	// TODO These can return void
	OScDev_Error (*Acquisition_IsClockRequested)(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *acq, bool *isRequested);
	OScDev_Error (*Acquisition_IsScannerRequested)(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *acq, bool *isRequested);
	OScDev_Error (*Acquisition_IsDetectorRequested)(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *acq, bool *isRequested);
	OScDev_Error (*Acquisition_GetClockStartTriggerSource)(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *acq, OScDev_TriggerSource *startTrigger);
	OScDev_Error (*Acquisition_GetClockSource)(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *acq, OScDev_ClockSource *clock);

	uint32_t (*Acquisition_GetNumberOfFrames)(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *acq);
	double (*Acquisition_GetPixelRate)(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *acq);
	uint32_t (*Acquisition_GetResolution)(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *acq);
	double (*Acquisition_GetZoomFactor)(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *acq);
	void (*Acquisition_GetROI)(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *acq,
		uint32_t *xOffset, uint32_t *yOffset, uint32_t *width, uint32_t *height);

	bool (*Acquisition_CallFrameCallback)(OScDev_ModuleImpl *modImpl, OScDev_Acquisition *acq, uint32_t channel, void *pixels);
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

	/// If the device module supports RichErrors.
	/*
	* To do.
	*/
	const bool supportsRichErrors;

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

	/// Return the device implementations available in this module.
	/**
	 * A device implementation defines the code to handle a particular type of
	 * device. Many modules will only have one device implementation.
	 *
	 * **Required**
	 *
	 * `deviceImpls` must be an array of `struct` ::OScDev_DeviceImpl objects.
	 * OpenScanLib takes ownership of the array (unless the array is statically
	 * defined). The device implementation objects must remain valid while this
	 * module remains loaded (i.e. indefinitely since we do not support
	 * unloading).
	 */
	OScDev_Error (*GetDeviceImpls)(OScDev_PtrArray **deviceImpls);
};


struct OScDev_DeviceImpl
{
	OScDev_Error (*GetModelName)(const char **name);

	/// Create instances for each physically available device.
	/**
	 * Implement this function if it is possible to automatically detect the
	 * available devices on the computer. It is also acceptable to implement
	 * this function when the device driver supports a small and fixed number
	 * of devices that can be easily listed.
	 *
	 * **Required**
	 *
	 * \todo Provide an alternative mechanism to instantiate devices that
	 * cannot be enumerated. Rather than imitating Micro-Manager's leaky
	 * "pre-init property" mechanism, we should use dedicated "device loader"
	 * objects, which can have settings such as port numbers.
	 *
	 * `devices` must be an array of OScDev_Device objects. OpenScanLib takes
	 * ownership of the array (unless the array is statically defined) as well
	 * as the device instances. Therefore, the device instances must be created
	 * newly every time this function is called, and ReleaseInstance must be
	 * correctly implemented.
	 *
	 * It is OpenScanLib's responsibility to ensure that no more than 1
	 * instance of the same device (as identified by `GetName`) is ever opened
	 * at the same time.
	 */
	OScDev_Error (*EnumerateInstances)(OScDev_PtrArray **devices);

	/// Free the private data for a device.
	/**
	 * If this device implementation passes non-NULL data to
	 * OScDev_Device_Create(), it must also implement this function to free
	 * the private data and any resources referenced from it.
	 *
	 * OpenScanLib ensures that the device is closed (if it was ever opened)
	 * before this function is called.
	 *
	 * **Conditionally required**
	 */
	OScDev_Error (*ReleaseInstance)(OScDev_Device *device);

	OScDev_Error (*GetName)(OScDev_Device *device, char *name);

	/// Establish a connection to the device.
	/**
	 * Importantly, a device instance must not connect to the physical device
	 * until this function is called. This is because multiple instances may be
	 * created (but not opened) for the same device.
	 */
	OScDev_Error (*Open)(OScDev_Device *device);

	OScDev_Error (*Close)(OScDev_Device *device);

	/// Return true if this device can provide the clock for scanning and detection.
	/**
	 * The clock can be any reference against which the scanning and detection
	 * are synchronized. When the scanner and detector belong to the same device,
	 * the clock can be entirely internal. When the scanner and detector belong
	 * to separate devices, typically one or the other device also provides the
	 * clock, and the other device receives the clock through a hardware
	 * connection (most commonly a TTL line clock, but this need not be the
	 * case).
	 */
	OScDev_Error (*HasClock)(OScDev_Device *device, bool *hasClock);

	/// Return true if this device can perform scanning.
	OScDev_Error (*HasScanner)(OScDev_Device *device, bool *hasScanner);

	/// Return true if this device can perform detection.
	OScDev_Error (*HasDetector)(OScDev_Device *device, bool *hasDetector);

	/// Create the settings for a device.
	OScDev_Error (*MakeSettings)(OScDev_Device *device, OScDev_PtrArray **settings);

	/// Return the allowed pixel rates.
	/**
	 * The range returned in `pixelRatesHz` must be stateless, i.e., must not
	 * change over the lifetime of the device.
	 *
	 * Pixel rate applies to the clock, scanner, and detector.
	 *
	 * **Required**
	 */
	OScDev_Error (*GetPixelRates)(OScDev_Device *device, OScDev_NumRange **pixelRatesHz);

	/// Return the actual pixel rate with best known accuracy.
	/**
	 * If the device supports a range (rather than discrete values) of pixel
	 * rates but is known to modify the requested pixel rate (for example,
	 * because the pixel rate is generated by frequency division), this
	 * function should be implemented to compute the actual pixel rate at the
	 * best available precision.
	 *
	 * This information is used to match pixel rates between the scanner and
	 * detector devices and/or warn the user of unmatched pixel rates.
	 *
	 * **Optional**; if not implemented, it is assumed that the device can
	 * operate at the exact requested pixel rate, up to a reasonable precision,
	 * or that matching of pixel rate is not critical because a hardware
	 * (internal or external) pixel clock signal (or equivalent) is used.
	 *
	 * \todo It is not clear if this is the best method of matching pixel
	 * rates. At the very least, this does allow OpenScanLib to quantify the
	 * error, so that the user might establish a discrete set of usable rates.
	 * That's probably better than any sort of poorly thought-through
	 * automagical matching.
	 */
	OScDev_Error (*GetActualPixelRate)(OScDev_Device *device, double nominalRateHz, double *actualRateHz);

	/// Return the allowed scanner resolutions.
	/**
	 * The range returned in `resolutions` must be stateless, i.e., must not
	 * change over the lifetime of the device.
	 *
	 * Resolution only applies to the scanner. (The clock and detector operate
	 * based on raster size, not resolution.)
	 *
	 * **Required** if the device has a scanner.
	 */
	OScDev_Error (*GetResolutions)(OScDev_Device *device, OScDev_NumRange **resolutions);

	/// Return the allowed zoom factors.
	/**
	 * The range returned in `zooms` must be stateless, i.e., must not change
	 * over the lifetime of the device. The range should include the zoom
	 * factor of 1.0.
	 *
	 * Zoom factor only applies to the scanner.
	 *
	 * **Optional**; the default implementation will allow only a zoom of 1.0.
	 */
	OScDev_Error (*GetZoomFactors)(OScDev_Device *device, OScDev_NumRange **zooms);

	/// Return whether the scanner can perform subregion scans.
	/**
	 * The value returned in `supported` must be stateless, i.e., must not
	 * change over the lifetime of the device.
	 *
	 * The scanner must always support a full-frame scan (width and height
	 * equal to resolution). If, in addition, it supports scanning (nonempty)
	 * rectangular subregions, this function should be implemented and return
	 * true.
	 *
	 * A scanner that supports ROI scans must allow the ROI to start at an
	 * arbitrary offset (`0` to `(resolution - 1)` in both X and Y). The size
	 * (width and height) of the ROI conforms to ranges returned by
	 * GetRasterWidths() and GetRasterHeights(), limited by the resolution.
	 *
	 * **Optional**; the default implementation will return false.
	 */
	OScDev_Error (*IsROIScanSupported)(OScDev_Device *device, bool *supported);

	/// Return the allowed widths of the raster ROI.
	/**
	 * The range returned in `widths` must be stateless, i.e., must not change
	 * over the lifetime of the device.
	 *
	 * Raster size applies to the clock, scanner, and detector.
	 *
	 * **Optional**; the default implementation will allow all positive integer
	 * widths. If the device only works with an internal scanner, the range
	 * will be limited by the scanner resolution.
	 */
	OScDev_Error (*GetRasterWidths)(OScDev_Device *device, OScDev_NumRange **widths);

	/// Return the allowed heights of the raster ROI.
	/**
	 * The range returned in `heights` must be stateless, i.e., must not change
	 * over the lifetime of the device.
	 *
	 * Raster size applies to the clock, scanner, and detector.
	 *
	 * **Optional**; the default implementation will allow all positive integer
	 * widths. If the device only works with an internal scanner, the range
	 * will be limited by the scanner resolution.
	 */
	OScDev_Error (*GetRasterHeights)(OScDev_Device *device, OScDev_NumRange **heights);

	/// Return the number of channels given current settings.
	/**
	 * Currently, only simultaneous multi-channel imaging is supported, so this
	 * function is only called for the device providing the detector.
	 *
	 * **Required** if this device has a detector.
	 */
	OScDev_Error (*GetNumberOfChannels)(OScDev_Device *device, uint32_t *nChannels);

	/// Return the bytes per sample given current settings
	/**
	 * Values other than 2 (16-bit) are not currently supported.
	 *
	 * **Required** if this device has a detector.
	 */
	OScDev_Error (*GetBytesPerSample)(OScDev_Device *device, uint32_t *bytesPerSample);

	/// Prepare the clock, scanner, and/or detector for an acquisition.
	/**
	 * An implementation of this function should first query the acquisition
	 * object `acq` to learn whether the device should provide the clock,
	 * scanner, and/or detector for the acquisition.
	 *
	 * If providing the clock, the clock should be set up to immediately start
	 * upon receiving the start trigger. The start trigger may be external
	 * (hardware input) or software (a call to the `Start` function); call
	 * OScDev_Acquisition_GetClockStartTriggerSource() to determine which.
	 *
	 * If providing the scanner, the scanner should be set up to scan according
	 * to clock signals. The clock may be internal (i.e. provided by this same
	 * device) or external (hardware input); call
	 * OScDev_Acquisition_GetClockSource() to determine which.
	 *
	 * If providing the detector, the detector should be set up to acquire
	 * according to clock signals. The clock may be internal (i.e. provided by
	 * this same device) or external (hardware input); call
	 * OScDev_Acquisition_GetClockSource() to determine which.
	 *
	 * In some integrated devices, the separation between the clock, scanner,
	 * and detector may not be clear. The implementation should behave _as if_
	 * the three components were separate and behaved as specified above.
	 *
	 * \todo Currently, state change after the device is armed is handled in a
	 * variable way depending on what triggers the state change (especially
	 * stop of acquisition). We should replace this with a simpler interface
	 * where the device calls new functions `OScDev_Acquisition_NotifyStart()`
	 * and `OScDev_Acquisition_NotifyStop()`. The start notification will be
	 * purely informational, used e.g. so that the GUI can show "waiting for
	 * start trigger" vs "acquisition running". The stop trigger will indicate
	 * that all action has finished and the device is ready to be disarmed.
	 * (`Disarm()` should be a mandatory function distinct from `Stop()`, which
	 * should behave as a software trigger for cancellation.) There should also
	 * be a way for a device to declare that it is not capable of issuing a
	 * start notification (in which case arrival of data can be used as a proxy
	 * for display purposes). `Stop()`, unlike `Disarm()`, need not be called
	 * when OpenScanLib has already received a stop notification.
	 */
	OScDev_Error (*Arm)(OScDev_Device *device, OScDev_Acquisition *acq);

	/// Start the clock from software.
	/**
	 * A call to this function is how a software start trigger to the clock is
	 * implemented.
	 *
	 * Before this function is called, `Arm()` would have been called with the
	 * clock source set to `OScDev_ClockSource_Internal` and the start trigger
	 * source set to `OScDev_TriggerSource_Software`.
	 *
	 * It should be noted that a device that is not providing the clock will
	 * _not_ receive a call to this function.
	 *
	 * \todo Better to rename to `SoftwareTriggerStart()`?
	 */
	OScDev_Error (*Start)(OScDev_Device *device);

	/// Stop the current acquisition and disarm.
	/**
	 * Unlike `Start`, this function is called for all devices participating in
	 * an acquisition, not just the device providing the clock. In this sense,
	 * `Stop` is the counterpart of `Arm`, not `Start`.
	 *
	 * If there is no acquisition running and the device is not armed, then this
	 * function must succeed with no effect.
	 *
	 * Currently, this function must wait for the stopping process to complete
	 * and return only when the device is ready to be armed again with a new
	 * acquisition.
	 *
	 * \todo This function should be split into two separate functions,
	 * `SoftwareTriggerStop()` and `Disarm()`, with very different roles.
	 * The former should merely be an asynchronous signal that the device
	 * should ignore any subsequent start triggers and stop as soon as
	 * possible. Actual completion of stopping should be notified via a
	 * callback (see Todo at Arm). The latter (`Disarm()`) should mainly
	 * handle cleanup, and should be guaranteed to be called exactly once per
	 * armed acquisition, after all processes have stopped.
	 * \todo `SoftwareTriggerStop()` can be optional to implement; we should
	 * additionally provide `bool OScDev_Acquisition_SoftwareStopTriggered()`
	 * which can be called any time between `Arm()` and `Disarm()`; this will
	 * supersede the `bool` return value of
	 * `OScDev_Acquisition_CallFrameCalback()`.
	 */
	OScDev_Error (*Stop)(OScDev_Device *device);

	/// Return true if this device is armed or running an acquisition.
	/**
	 * Note that the device may exit the "running" state on its own (e.g. after
	 * completing a specified number of frames), or as the result of a call to
	 * `StopAcquisition()`.
	 *
	 * In all cases, this function should not return `false` until all aspects
	 * of the acquisition have completed and the device is ready to arm for a new
	 * acquisition.
	 *
	 * \todo This function should be replaced with a callback-based interface
	 * to avoid polling and simplify device module implementation.
	 */
	OScDev_Error (*IsRunning)(OScDev_Device *device, bool *isRunning);

	/// Wait for any current acquisition to finish.
	/**
	 * As soon as this function returns, `IsRunning()` must return `false` until
	 * `Arm()` is called again.
	 *
	 * This function should return immediately if `IsRunning()` is already false.
	 *
	 * \todo This function should be removed, because it leads to reinvention of
	 * condition variable code in each device module. Better to report state
	 * changes using a callback interface.
	 */
	OScDev_Error (*Wait)(OScDev_Device *device);
};


struct OScDev_SettingImpl
{
	// TODO The value type should be moved inside the SettingImpl as a static
	// data field. The type-dependent fields could be placed in a union:
	//     .valueType = OScDev_ValueType_Int32,
	//     .typeImpl.int32 = {
	//         .constraintType = OScDev_ValueConstraint_Range,
	//         .Get = myGetFunc,
	//         .Set = mySetFunc,
	//         .GetRange = myRangeFunc,
	//     }
	// We can also provide static data fields for range and discrete values.

	OScDev_Error (*IsEnabled)(OScDev_Setting *setting, bool *enabled);
	OScDev_Error (*IsWritable)(OScDev_Setting *setting, bool *writable);

	/**
	 * \brief Get the type of value constraint of an int32 or float64 setting.
	 *
	 * \todo We cannot realistically allow the constraint type to change, so
	 * this should be a static data field.
	 */
	OScDev_Error (*GetNumericConstraintType)(OScDev_Setting *setting, OScDev_ValueConstraint *constraintType);
	OScDev_Error (*GetString)(OScDev_Setting *setting, char *value);
	OScDev_Error (*SetString)(OScDev_Setting *setting, const char *value);
	OScDev_Error (*GetBool)(OScDev_Setting *setting, bool *value);
	OScDev_Error (*SetBool)(OScDev_Setting *setting, bool value);
	OScDev_Error (*GetInt32)(OScDev_Setting *setting, int32_t *value);
	OScDev_Error (*SetInt32)(OScDev_Setting *setting, int32_t value);

	/**
	 * \brief Get the range of an int32 setting with range constraint.
	 *
	 * **Required** if value type is int32 and numeric constraint type is
	 * range.
	 *
	 * \pre \p setting, \p min, and \p max are guaranteed to be valid pointers.
	 */
	OScDev_Error (*GetInt32Range)(OScDev_Setting *setting, int32_t *min, int32_t *max);

	/**
	 * \brief Get the allowed values of an int32 setting with discrete values
	 * constraint.
	 *
	 * **Required** if value type is int32 and numeric constraint type is
	 * discrete values.
	 *
	 * \pre \p setting and \p values are guaranteed to be valid pointers.
	 *
	 * The implementation must create a new ::OScDev_NumArray and set `*values`
	 * to its address.
	 *
	 * \param setting the setting
	 * \param[out] values address where implementation should write pointer to
	 * new array containing allowed values (which must fit in int32) for the
	 * setting
	 * \return error in case hardware need to be queried and query failed
	 */
	OScDev_Error (*GetInt32DiscreteValues)(OScDev_Setting *setting, OScDev_NumArray **values);

	OScDev_Error (*GetFloat64)(OScDev_Setting *setting, double *value);
	OScDev_Error (*SetFloat64)(OScDev_Setting *setting, double value);

	/**
	 * \brief Get the range of an float64 setting with range constraint.
	 *
	 * **Required** if value type is float64 and numeric constraint type is
	 * range.
	 *
	 * \pre \p setting, \p min, and \p max are guaranteed to be valid pointers.
	 */
	OScDev_Error (*GetFloat64Range)(OScDev_Setting *setting, double *min, double *max);

	/**
	 * \brief Get the allowed values of an float64 setting with discrete values
	 * constraint.
	 *
	 * **Required** if value type is float64 and numeric constraint type is
	 * discrete values.
	 *
	 * \pre \p setting and \p values are guaranteed to be valid pointers.
	 *
	 * The implementation must create a new ::OScDev_NumArray and set `*values`
	 * to its address.
	 *
	 * \param setting the setting
	 * \param[out] values address where implementation should write pointer to
	 * new array containing allowed values for the setting
	 * \return error in case hardware need to be queried and query failed
	 */
	OScDev_Error (*GetFloat64DiscreteValues)(OScDev_Setting *setting, OScDev_NumArray **values);
	OScDev_Error (*GetEnum)(OScDev_Setting *setting, uint32_t *value);
	OScDev_Error (*SetEnum)(OScDev_Setting *setting, uint32_t value);
	OScDev_Error (*GetEnumNumValues)(OScDev_Setting *setting, uint32_t *count);
	OScDev_Error (*GetEnumNameForValue)(OScDev_Setting *setting, uint32_t value, char *name);
	OScDev_Error (*GetEnumValueForName)(OScDev_Setting *setting, uint32_t *value, const char *name);

	/// Free the private data associated with a setting, if any.
	void (*Release)(OScDev_Setting *setting);
};


#ifdef __cplusplus
} // extern "C"
#endif

#ifndef OScDevInternal_BUILDING_OPENSCANLIB

// Inline functions wrapping all entries in the OpenScanLib function table
//
// Use of this macro tricks Doxygen into documenting the functions even though
// they are static.
#define OScDev_API static inline

/** \addtogroup dpi
 * @{
 */

/// Log a message
OScDev_API void OScDev_Log(OScDev_Device *device, OScDev_LogLevel level, const char *message)
{
	OScDevInternal_FunctionTable->Log(&OScDevInternal_TheModuleImpl, device, level, message);
}

/// Log a debug-level message
OScDev_API void OScDev_Log_Debug(OScDev_Device *device, const char *message)
{
	OScDev_Log(device, OScDev_LogLevel_Debug, message);
}

/// Log an info-level message
OScDev_API void OScDev_Log_Info(OScDev_Device *device, const char *message)
{
	OScDev_Log(device, OScDev_LogLevel_Info, message);
}

/// Log a warning-level message
OScDev_API void OScDev_Log_Warning(OScDev_Device *device, const char *message)
{
	OScDev_Log(device, OScDev_LogLevel_Warning, message);
}

/// Log an error-level message
OScDev_API void OScDev_Log_Error(OScDev_Device *device, const char *message)
{
	OScDev_Log(device, OScDev_LogLevel_Error, message);
}

// APIs for device modules
OScDev_API OScDev_RichError *OScDev_Error_RegisterCodeDomain(const char* domainName, OScDev_ErrorCodeFormat codeFormat) {
	return OScDevInternal_FunctionTable->Error_RegisterCodeDomain(&OScDevInternal_TheModuleImpl, domainName, codeFormat);
}

OScDev_API OScDev_Error OScDev_Error_ReturnAsCode(OScDev_RichError *error) {
	return OScDevInternal_FunctionTable->Error_ReturnAsCode(&OScDevInternal_TheModuleImpl, error);
}

/// Create an array of objects.
OScDev_API OScDev_PtrArray *OScDev_PtrArray_Create(void)
{
	return OScDevInternal_FunctionTable->PtrArray_Create(&OScDevInternal_TheModuleImpl);
}

OScDev_API OScDev_PtrArray *OScDev_PtrArray_CreateFromNullTerminated(void *const *nullTerminatedArray)
{
	return OScDevInternal_FunctionTable->PtrArray_CreateFromNullTerminated(&OScDevInternal_TheModuleImpl, nullTerminatedArray);
}

/// Destroy (free) an array of objects.
OScDev_API void OScDev_PtrArray_Destroy(const OScDev_PtrArray *arr)
{
	OScDevInternal_FunctionTable->PtrArray_Destroy(&OScDevInternal_TheModuleImpl, arr);
}

/// Append an object to an array.
OScDev_API void OScDev_PtrArray_Append(OScDev_PtrArray *arr, void *obj)
{
	OScDevInternal_FunctionTable->PtrArray_Append(&OScDevInternal_TheModuleImpl, arr, obj);
}

OScDev_API size_t OScDev_PtrArray_Size(const OScDev_PtrArray *arr)
{
	return OScDevInternal_FunctionTable->PtrArray_Size(&OScDevInternal_TheModuleImpl, arr);
}

OScDev_API bool OScDev_PtrArray_Empty(const OScDev_PtrArray *arr)
{
	return OScDevInternal_FunctionTable->PtrArray_Empty(&OScDevInternal_TheModuleImpl, arr);
}

OScDev_API void *OScDev_PtrArray_At(const OScDev_PtrArray *arr, size_t index)
{
	return OScDevInternal_FunctionTable->PtrArray_At(&OScDevInternal_TheModuleImpl, arr, index);
}

/// Create an array of numbers.
OScDev_API OScDev_NumArray *OScDev_NumArray_Create(void)
{
	return OScDevInternal_FunctionTable->NumArray_Create(&OScDevInternal_TheModuleImpl);
}

OScDev_API OScDev_NumArray *OScDev_NumArray_CreateFromNaNTerminated(const double *nanTerminatedArray)
{
	return OScDevInternal_FunctionTable->NumArray_CreateFromNaNTerminated(&OScDevInternal_TheModuleImpl, nanTerminatedArray);
}

/// Destroy (free) an array of numbers.
OScDev_API void OScDev_NumArray_Destroy(const OScDev_NumArray *arr)
{
	OScDevInternal_FunctionTable->NumArray_Destroy(&OScDevInternal_TheModuleImpl, arr);
}

/// Append a value to an array.
OScDev_API void OScDev_NumArray_Append(OScDev_NumArray *arr, double val)
{
	OScDevInternal_FunctionTable->NumArray_Append(&OScDevInternal_TheModuleImpl, arr, val);
}

OScDev_API size_t OScDev_NumArray_Size(const OScDev_NumArray *arr)
{
	return OScDevInternal_FunctionTable->NumArray_Size(&OScDevInternal_TheModuleImpl, arr);
}

OScDev_API bool OScDev_NumArray_Empty(const OScDev_NumArray *arr)
{
	return OScDevInternal_FunctionTable->NumArray_Empty(&OScDevInternal_TheModuleImpl, arr);
}

OScDev_API double OScDev_NumArray_At(const OScDev_NumArray *arr, size_t index)
{
	return OScDevInternal_FunctionTable->NumArray_At(&OScDevInternal_TheModuleImpl, arr, index);
}

OScDev_API OScDev_NumRange *OScDev_NumRange_CreateContinuous(double rMin, double rMax)
{
	return OScDevInternal_FunctionTable->NumRange_CreateContinuous(&OScDevInternal_TheModuleImpl, rMin, rMax);
}

OScDev_API OScDev_NumRange *OScDev_NumRange_CreateDiscrete(void)
{
	return OScDevInternal_FunctionTable->NumRange_CreateDiscrete(&OScDevInternal_TheModuleImpl);
}

OScDev_API OScDev_NumRange *OScDev_NumRange_CreateDiscreteFromNaNTerminated(const double *nanTerminatedArray)
{
	return OScDevInternal_FunctionTable->NumRange_CreateDiscreteFromNaNTerminated(&OScDevInternal_TheModuleImpl, nanTerminatedArray);
}

OScDev_API void OScDev_NumRange_Destroy(const OScDev_NumRange *range)
{
	OScDevInternal_FunctionTable->NumRange_Destroy(&OScDevInternal_TheModuleImpl, range);
}

OScDev_API void OScDev_NumRange_AppendDiscrete(OScDev_NumRange *range, double value)
{
	OScDevInternal_FunctionTable->NumRange_AppendDiscrete(&OScDevInternal_TheModuleImpl, range, value);
}

OScDev_API OScDev_Error OScDev_Device_Create(OScDev_Device **device, OScDev_DeviceImpl *impl, void *data)
{
	return OScDevInternal_FunctionTable->Device_Create(&OScDevInternal_TheModuleImpl, device, impl, data);
}

OScDev_API void *OScDev_Device_GetImplData(OScDev_Device *device)
{
	return OScDevInternal_FunctionTable->Device_GetImplData(&OScDevInternal_TheModuleImpl, device);
}

OScDev_API OScDev_Error OScDev_Setting_Create(OScDev_Setting **setting, const char *name, OScDev_ValueType valueType, OScDev_SettingImpl *impl, void *data)
{
	return OScDevInternal_FunctionTable->Setting_Create(&OScDevInternal_TheModuleImpl, setting, name, valueType, impl, data);
}

OScDev_API void OScDev_Setting_Destroy(OScDev_Setting *setting)
{
	OScDevInternal_FunctionTable->Setting_Destroy(&OScDevInternal_TheModuleImpl, setting);
}

OScDev_API void *OScDev_Setting_GetImplData(OScDev_Setting *setting)
{
	return OScDevInternal_FunctionTable->Setting_GetImplData(&OScDevInternal_TheModuleImpl, setting);
}

/// Determine whether this device should provide the clock for the given acquisition.
OScDev_API OScDev_Error OScDev_Acquisition_IsClockRequested(OScDev_Acquisition *acq, bool *isRequested)
{
	return OScDevInternal_FunctionTable->Acquisition_IsClockRequested(&OScDevInternal_TheModuleImpl, acq, isRequested);
}

/// Determine whether this device should perform scanning for the given acquisition.
OScDev_API OScDev_Error OScDev_Acquisition_IsScannerRequested(OScDev_Acquisition *acq, bool *isRequested)
{
	return OScDevInternal_FunctionTable->Acquisition_IsScannerRequested(&OScDevInternal_TheModuleImpl, acq, isRequested);
}

/// Determine whether this device should perform detection for the given acquisition.
OScDev_API OScDev_Error OScDev_Acquisition_IsDetectorRequested(OScDev_Acquisition *acq, bool *isRequested)
{
	return OScDevInternal_FunctionTable->Acquisition_IsDetectorRequested(&OScDevInternal_TheModuleImpl, acq, isRequested);
}

/// Determine the start trigger source for the clock for the given acquisition.
/**
 * This function is only useful when the clock is requested for the acquisition
 * (see `OScDev_Acquisition_IsClockRequested()`).
 *
 * \param[in] acq the acquisition
 * \param[out] startTrigger the clock start trigger; either software or external
 */
OScDev_API OScDev_Error OScDev_Acquisition_GetClockStartTriggerSource(OScDev_Acquisition *acq, OScDev_TriggerSource *startTrigger)
{
	return OScDevInternal_FunctionTable->Acquisition_GetClockStartTriggerSource(&OScDevInternal_TheModuleImpl, acq, startTrigger);
}

/// Determine the scanner and detector clock source for the given acquisition.
/**
 * This function is only useful when the scanner and/or detector is requested
 * for the acquisition (see `OScDev_Acquisition_IsScannerRequested()` and
 * `OScDev_Acquisition_IsDetectorRequested()`).
 *
 * \todo This is redundant with OScDev_Acquisition_IsClockRequested(). Remove.
*/
OScDev_API OScDev_Error OScDev_Acquisition_GetClockSource(OScDev_Acquisition *acq, OScDev_ClockSource *clock)
{
	return OScDevInternal_FunctionTable->Acquisition_GetClockSource(&OScDevInternal_TheModuleImpl, acq, clock);
}

/// Determine the requested number of frames for the given acquisition.
OScDev_API uint32_t OScDev_Acquisition_GetNumberOfFrames(OScDev_Acquisition *acq)
{
	return OScDevInternal_FunctionTable->Acquisition_GetNumberOfFrames(&OScDevInternal_TheModuleImpl, acq);
}

OScDev_API double OScDev_Acquisition_GetPixelRate(OScDev_Acquisition *acq)
{
	return OScDevInternal_FunctionTable->Acquisition_GetPixelRate(&OScDevInternal_TheModuleImpl, acq);
}

OScDev_API uint32_t OScDev_Acquisition_GetResolution(OScDev_Acquisition *acq)
{
	return OScDevInternal_FunctionTable->Acquisition_GetResolution(&OScDevInternal_TheModuleImpl, acq);
}

OScDev_API double OScDev_Acquisition_GetZoomFactor(OScDev_Acquisition *acq)
{
	return OScDevInternal_FunctionTable->Acquisition_GetZoomFactor(&OScDevInternal_TheModuleImpl, acq);
}

OScDev_API void OScDev_Acquisition_GetROI(OScDev_Acquisition *acq,
	uint32_t *xOffset, uint32_t *yOffset, uint32_t *width, uint32_t *height)
{
	OScDevInternal_FunctionTable->Acquisition_GetROI(&OScDevInternal_TheModuleImpl, acq,
		xOffset, yOffset, width, height);
}

/// Send acquired data for one channel of a frame.
/**
 * This function must be called during an acquisition by the device that owns
 * the detector for the acquisition.
 *
 * The data pointed to by `pixels` must not change during the call to this
 * function.
 *
 * \param[in] acq the acquisition that was given when arming this device
 * \param[in] channel the channel index
 * \param[in] pixels the raw pixel data for the channel
 */
OScDev_API bool OScDev_Acquisition_CallFrameCallback(OScDev_Acquisition *acq, uint32_t channel, void *pixels)
{
	return OScDevInternal_FunctionTable->Acquisition_CallFrameCallback(&OScDevInternal_TheModuleImpl, acq, channel, pixels);
}

#undef OScDev_API

#endif // OScDevInternal_BUILDING_OPENSCANLIB

/** @} */ // addtogroup dpi
