/** \file
 * \brief Internal definitions for OpenScanDeviceLib.
 *
 * This header is NOT to be used by device modules, but defines the interface
 * by which OpenScanLib interfaces with device modules.
 */

#pragma once

#include "OpenScanDeviceLib.h"


#ifdef __cplusplus
extern "C" {
#endif


/** \addtogroup internal
 * @{
 */


#ifdef OScDevInternal_BUILDING_OPENSCANLIB
#	define OScDevInternal_ENTRY_POINT_EXPORT
#else
#	ifdef _MSC_VER
#		define OScDevInternal_ENTRY_POINT_EXPORT __declspec(dllexport)
#	else
#		error Not implemented for this platform.
#	endif
#endif // OScDev_IMPORT


/// Name of the device module entry point function.
/**
 * The name includes a version number in case we want to change the signature
 * or semantics of the entry point. OpenScanLib will only search for entry
 * point names with version number(s) it knows about.
 */
#define OScDevInternal_ENTRY_POINT OScDev_Module_EntryPoint_v0


// See the GCC manual under "Stringification" for an explanation of this trick
#define OScDevInternal_STRINGIFY_EXPANSION(s) OScDevInternal_STRINGIFY(s)
#define OScDevInternal_STRINGIFY(s) #s


/// Name of the entry point, used to obtain its address
#define OScDevInternal_ENTRY_POINT_NAME OScDevInternal_STRINGIFY_EXPANSION(OScDevInternal_ENTRY_POINT)


/// Entry point of device module.
/**
 * This function, defined by OpenScanDeviceLib, is called by OpenScanLib after
 * loading the device module. It allows OpenScanLib to check the module's
 * device interface version and obtain the module's implementation function
 * table.
 *
 * \param[out] devif set to the address of the module's (static) pointer to
 *                   the device interface function table (after this function
 *                   returns, OpenScanLib will set the pointer)
 * \param[out] impl set to the address of the module's implementation function
 *                  table
 * \returns the module's device interface version
 */
uint32_t OScDevInternal_ENTRY_POINT_EXPORT OScDevInternal_ENTRY_POINT(
	struct OScDevInternal_Interface ***devif,
	OScDev_ModuleImpl **impl);


/// Pointer to module entry point function.
typedef uint32_t (*OScDevInternal_EntryPointPtr)(
	struct OScDevInternal_Interface ***devif,
	OScDev_ModuleImpl **impl);


/** @} */ // addtogroup internal


#ifdef __cplusplus
} // extern "C"
#endif
