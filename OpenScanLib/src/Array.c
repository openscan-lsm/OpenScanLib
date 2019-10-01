#include "OpenScanLibPrivate.h"

#include <string.h>


OScDev_PtrArray *OSc_PtrArray_Create(void)
{
	OScDev_PtrArray *ret = calloc(1, sizeof(OScDev_PtrArray));
	if (!ret) {
		return NULL;
	}
	ret->isDynamic = true;
	return ret;
}


OScDev_NumArray *OSc_NumArray_Create(void)
{
	OScDev_NumArray *ret = calloc(1, sizeof(OScDev_NumArray));
	if (!ret) {
		return NULL;
	}
	ret->isDynamic = true;
	return ret;
}


void OSc_PtrArray_Destroy(const OScDev_PtrArray *arr)
{
	if (!arr->isDynamic) {
		return;
	}

	free(arr->ptr);

	// Clear for safety (this marks the array static, which is harmless because
	// it will no longer be used as a dynamically allocated array).
	memset((void *)arr, 0, sizeof(OScDev_PtrArray));
}


void OSc_NumArray_Destroy(const OScDev_NumArray *arr)
{
	if (!arr->isDynamic) {
		return;
	}

	free(arr->ptr);

	memset((void *)arr, 0, sizeof(OScDev_NumArray));
}


static size_t CapForSize(size_t size)
{
	size_t newCap = 16;
	while (size > newCap && newCap < 1024) {
		newCap *= 2;
	}
	while (size > newCap) {
		newCap += 1024;
	}
	return newCap;
}


void OSc_PtrArray_Append(OScDev_PtrArray *arr, void *obj)
{
	// We do not currently report errors. All errors that may occur are severe
	// (either out of memory or a programming error), so logging and crashing
	// would be a good way to handle. (Returning an error will only complicate
	// device module code or get ignored.)

	if (!arr->isDynamic) {
		return; // Programming error: cannot modify static PtrArray
	}

	size_t newSize = arr->size + 1;
	if (newSize > arr->capacity) {
		size_t newCap = CapForSize(newSize);
		size_t newCapBytes = sizeof(void *) * newCap;
		void *newPtr;
		if (!arr->ptr) {
			newPtr = malloc(newCapBytes);
		}
		else {
			newPtr = realloc(arr->ptr, newCapBytes);
		}
		if (!newPtr) {
			return; // Error: out of memory
		}

		arr->ptr = newPtr;
		arr->capacity = newCap;
	}

	arr->ptr[arr->size] = obj;
	arr->size = newSize;
}


void OSc_NumArray_Append(OScDev_NumArray *arr, double val)
{
	if (!arr->isDynamic) {
		return; // Programming error
	}

	size_t newSize = arr->size + 1;
	if (newSize > arr->capacity) {
		size_t newCap = CapForSize(newSize);
		size_t newCapBytes = sizeof(double) * newCap;
		double *newPtr;
		if (!arr->ptr) {
			newPtr = malloc(newCapBytes);
		}
		else {
			newPtr = realloc(arr->ptr, newCapBytes);
		}
		if (!newPtr) {
			return; // Out of memory
		}

		arr->ptr = newPtr;
		arr->capacity = newCap;
	}

	arr->ptr[arr->size] = val;
	arr->size = newSize;
}