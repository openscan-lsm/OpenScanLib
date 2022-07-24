#include "OpenScanLibPrivate.h"

#include <math.h>
#include <string.h>

struct OScInternal_PtrArray {
    void **ptr; // Array of void*
    size_t size;
    size_t capacity;
};

struct OScInternal_NumArray {
    double *ptr; // Array of double
    size_t size;
    size_t capacity;
};

struct OScInternal_NumContinuousRange {
    double rMin;
    double rMax;
};

struct OScInternal_NumRange {
    bool isList;
    union {
        struct OScInternal_NumArray list;
        struct OScInternal_NumContinuousRange range;
    } rep;
};

OScInternal_PtrArray *OScInternal_PtrArray_Create(void) {
    OScInternal_PtrArray *ret = calloc(1, sizeof(OScInternal_PtrArray));
    if (!ret) {
        return NULL;
    }
    return ret;
}

OScInternal_NumArray *OScInternal_NumArray_Create(void) {
    OScInternal_NumArray *ret = calloc(1, sizeof(OScInternal_NumArray));
    if (!ret) {
        return NULL;
    }
    return ret;
}

OScInternal_NumRange *OScInternal_NumRange_CreateContinuous(double rMin,
                                                            double rMax) {
    OScInternal_NumRange *ret = calloc(1, sizeof(OScInternal_NumRange));
    if (!ret) {
        return NULL;
    }
    ret->isList = false;
    ret->rep.range.rMin = rMin;
    ret->rep.range.rMax = rMax;
    return ret;
}

OScInternal_NumRange *OScInternal_NumRange_CreateDiscrete(void) {
    OScInternal_NumRange *ret = calloc(1, sizeof(OScInternal_NumRange));
    if (!ret) {
        return NULL;
    }
    ret->isList = true;
    return ret;
}

OScInternal_PtrArray *OScInternal_PtrArray_CreateFromNullTerminated(
    void *const *nullTerminatedArray) {
    OScInternal_PtrArray *ret = OScInternal_PtrArray_Create();

    if (!nullTerminatedArray)
        return ret;

    while (*nullTerminatedArray) {
        OScInternal_PtrArray_Append(ret, *nullTerminatedArray++);
    }
    return ret;
}

OScInternal_NumArray *OScInternal_NumArray_CreateFromNaNTerminated(
    const double *nanTerminatedArray) {
    OScInternal_NumArray *ret = OScInternal_NumArray_Create();

    if (!nanTerminatedArray)
        return ret;

    while (!isnan(*nanTerminatedArray)) {
        OScInternal_NumArray_Append(ret, *nanTerminatedArray++);
    }
    return ret;
}

OScInternal_NumRange *OScInternal_NumRange_CreateDiscreteFromNaNTerminated(
    const double *nanTerminatedArray) {
    OScInternal_NumRange *ret = OScInternal_NumRange_CreateDiscrete();

    if (!nanTerminatedArray)
        return ret;

    while (!isnan(*nanTerminatedArray)) {
        OScInternal_NumRange_AppendDiscrete(ret, *nanTerminatedArray++);
    }
    return ret;
}

OScInternal_NumArray *
OScInternal_NumArray_Copy(const OScInternal_NumArray *src) {
    OScInternal_NumArray *ret = OScInternal_NumArray_Create();
    ret->capacity = src->size;
    ret->ptr = malloc(src->size * sizeof(double));
    memcpy(ret->ptr, src->ptr, src->size * sizeof(double));
    ret->size = src->size;
    return ret;
}

void OScInternal_PtrArray_Destroy(const OScInternal_PtrArray *arr) {
    free(arr->ptr);

    // Clear for safety (this marks the array static, which is harmless because
    // it will no longer be used as a dynamically allocated array).
    memset((void *)arr, 0, sizeof(OScInternal_PtrArray));
}

void OScInternal_NumArray_Destroy(const OScInternal_NumArray *arr) {
    free(arr->ptr);

    memset((void *)arr, 0, sizeof(OScInternal_NumArray));
}

void OScInternal_NumRange_Destroy(const OScInternal_NumRange *range) {
    if (range->isList) {
        free(range->rep.list.ptr);
    }

    memset((void *)range, 0, sizeof(OScInternal_NumRange));
}

static size_t CapForSize(size_t size) {
    size_t newCap = 16;
    while (size > newCap && newCap < 1024) {
        newCap *= 2;
    }
    while (size > newCap) {
        newCap += 1024;
    }
    return newCap;
}

void OScInternal_PtrArray_Append(OScInternal_PtrArray *arr, void *obj) {
    // We do not currently report errors. All errors that may occur are severe
    // (either out of memory or a programming error), so logging and crashing
    // would be a good way to handle. (Returning an error will only complicate
    // device module code or get ignored.)

    size_t newSize = arr->size + 1;
    if (newSize > arr->capacity) {
        size_t newCap = CapForSize(newSize);
        size_t newCapBytes = sizeof(void *) * newCap;
        void *newPtr;
        if (!arr->ptr) {
            newPtr = malloc(newCapBytes);
        } else {
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

void OScInternal_NumArray_Append(OScInternal_NumArray *arr, double val) {
    size_t newSize = arr->size + 1;
    if (newSize > arr->capacity) {
        size_t newCap = CapForSize(newSize);
        size_t newCapBytes = sizeof(double) * newCap;
        double *newPtr;
        if (!arr->ptr) {
            newPtr = malloc(newCapBytes);
        } else {
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

void OScInternal_NumRange_AppendDiscrete(OScInternal_NumRange *range,
                                         double val) {
    if (!range->isList) {
        return; // Programming error
    }
    OScInternal_NumArray_Append(&range->rep.list, val);
}

static int CompareDouble(const void *pLhs, const void *pRhs) {
    // Following IEEE-754 totalOrder()

    int64_t lhs, rhs;
    memcpy(&lhs, pLhs, sizeof(double));
    memcpy(&rhs, pRhs, sizeof(double));

    lhs ^= (lhs >> 63) & INT64_MAX;
    rhs ^= (rhs >> 63) & INT64_MAX;

    if (lhs < rhs)
        return -1;
    if (lhs > rhs)
        return +1;
    return 0;
}

void OScInternal_NumArray_SortAscending(OScInternal_NumArray *arr) {
    qsort(arr->ptr, arr->size, sizeof(double), CompareDouble);
}

size_t OScInternal_PtrArray_Size(const OScInternal_PtrArray *arr) {
    if (!arr)
        return 0;
    return arr->size;
}

bool OScInternal_PtrArray_Empty(const OScInternal_PtrArray *arr) {
    return OScInternal_PtrArray_Size(arr) == 0;
}

size_t OScInternal_NumArray_Size(const OScInternal_NumArray *arr) {
    if (!arr)
        return 0;
    return arr->size;
}

bool OScInternal_NumArray_Empty(const OScInternal_NumArray *arr) {
    return OScInternal_NumArray_Size(arr) == 0;
}

void *OScInternal_PtrArray_At(const OScInternal_PtrArray *arr, size_t index) {
    if (!arr || index >= arr->size)
        return NULL;
    return arr->ptr[index];
}

double OScInternal_NumArray_At(const OScInternal_NumArray *arr, size_t index) {
    if (!arr || index >= arr->size)
        return NAN;
    return arr->ptr[index];
}

void *const *OScInternal_PtrArray_Data(const OScInternal_PtrArray *arr) {
    if (!arr)
        return NULL;
    return arr->ptr;
}

bool OScInternal_NumRange_IsDiscrete(const OScInternal_NumRange *range) {
    if (!range)
        return false;
    return range->isList;
}

OScInternal_NumArray *
OScInternal_NumRange_DiscreteValues(const OScInternal_NumRange *range) {
    if (!range || !range->isList)
        return NULL; // Programming error
    return OScInternal_NumArray_Copy(&range->rep.list);
}

double OScInternal_NumArray_Min(const OScInternal_NumArray *arr) {
    double min = INFINITY;
    for (size_t i = 0; i < arr->size; ++i) {
        if (arr->ptr[i] < min)
            min = arr->ptr[i];
    }
    return min;
}

double OScInternal_NumArray_Max(const OScInternal_NumArray *arr) {
    double max = -INFINITY;
    for (size_t i = 0; i < arr->size; ++i) {
        if (arr->ptr[i] > max)
            max = arr->ptr[i];
    }
    return max;
}

double OScInternal_NumRange_Min(const OScInternal_NumRange *range) {
    if (!range)
        return NAN;
    if (range->isList) {
        return OScInternal_NumArray_Min(&range->rep.list);
    } else {
        return range->rep.range.rMin;
    }
}

double OScInternal_NumRange_Max(const OScInternal_NumRange *range) {
    if (!range)
        return NAN;
    if (range->isList) {
        return OScInternal_NumArray_Min(&range->rep.list);
    } else {
        return range->rep.range.rMax;
    }
}

double OScInternal_NumRange_ClosestValue(const OScInternal_NumRange *range,
                                         double value) {
    if (range->isList) {
        double minAbsDiff = INFINITY;
        double candidate = NAN;
        for (size_t i = 0; i < range->rep.list.size; ++i) {
            double absDiff = fabs(range->rep.list.ptr[i] - value);
            if (absDiff < minAbsDiff) {
                minAbsDiff = absDiff;
                candidate = range->rep.list.ptr[i];
            }
        }
        return candidate;
    } else {
        if (value < range->rep.range.rMin)
            return range->rep.range.rMin;
        if (value > range->rep.range.rMax)
            return range->rep.range.rMax;
        return value;
    }
}

bool OScInternal_NumRange_Contains(const OScInternal_NumRange *range,
                                   double value) {
    if (range->isList) {
        for (size_t i = 0; i < range->rep.list.size; ++i) {
            if (range->rep.list.ptr[i] == value)
                return true;
        }
        return false;
    }
    return value >= range->rep.range.rMin && value <= range->rep.range.rMax;
}

static OScInternal_NumRange *
DiscreteRangeIntersection(const OScInternal_NumRange *r1,
                          const OScInternal_NumRange *r2) {
    OScInternal_NumArray *a1 = OScInternal_NumRange_DiscreteValues(r1);
    OScInternal_NumArray *a2 = OScInternal_NumRange_DiscreteValues(r2);
    OScInternal_NumArray_SortAscending(a1);
    OScInternal_NumArray_SortAscending(a2);

    OScInternal_NumRange *ret = OScInternal_NumRange_CreateDiscrete();
    for (size_t i = 0, j = 0; i < a1->size && j < a2->size;) {
        if (a1->ptr[i] < a2->ptr[j]) {
            ++i;
        } else if (a1->ptr[i] > a2->ptr[j]) {
            ++j;
        } else {
            OScInternal_NumRange_AppendDiscrete(ret, a1->ptr[i]);
            ++i;
            ++j;
        }
    }

    OScInternal_NumArray_Destroy(a2);
    OScInternal_NumArray_Destroy(a1);
    return ret;
}

static OScInternal_NumRange *ClipDiscreteRange(const OScInternal_NumRange *r,
                                               double min, double max) {
    OScInternal_NumRange *ret = OScInternal_NumRange_CreateDiscrete();
    for (size_t i = 0; i < r->rep.list.size; ++i) {
        double v = r->rep.list.ptr[i];
        if (min <= v && v <= max) {
            OScInternal_NumRange_AppendDiscrete(ret, v);
        }
    }
    return ret;
}

static OScInternal_NumRange *
ContinuousRangeIntersection(const OScInternal_NumRange *r1,
                            const OScInternal_NumRange *r2) {
    if (r1->rep.range.rMax < r2->rep.range.rMin ||
        r2->rep.range.rMax < r1->rep.range.rMin) {
        return OScInternal_NumRange_CreateDiscrete(); // Empty
    }

    double retMin = r1->rep.range.rMin < r2->rep.range.rMin
                        ? r2->rep.range.rMin
                        : r1->rep.range.rMin;
    double retMax = r1->rep.range.rMax < r2->rep.range.rMax
                        ? r1->rep.range.rMax
                        : r2->rep.range.rMax;
    return OScInternal_NumRange_CreateContinuous(retMin, retMax);
}

OScInternal_NumRange *
OScInternal_NumRange_Intersection(const OScInternal_NumRange *r1,
                                  const OScInternal_NumRange *r2) {
    if (!r1 || !r2)
        return NULL;

    if (r1->isList && r2->isList) {
        return DiscreteRangeIntersection(r1, r2);
    }

    if (r1->isList) {
        const OScInternal_NumRange *wk = r1;
        r1 = r2;
        r2 = wk;
    }
    // r1 is now min-max
    if (r2->isList) {
        return ClipDiscreteRange(r2, r1->rep.range.rMin, r1->rep.range.rMax);
    } else {
        return ContinuousRangeIntersection(r1, r2);
    }
}

OScInternal_NumRange *
OScInternal_NumRange_Intersection3(const OScInternal_NumRange *r1,
                                   const OScInternal_NumRange *r2,
                                   const OScInternal_NumRange *r3) {
    OScInternal_NumRange *tmp = OScInternal_NumRange_Intersection(r1, r2);
    OScInternal_NumRange *ret = OScInternal_NumRange_Intersection(tmp, r3);
    OScInternal_NumRange_Destroy(tmp);
    return ret;
}

OScInternal_NumRange *OScInternal_NumRange_Intersection4(
    const OScInternal_NumRange *r1, const OScInternal_NumRange *r2,
    const OScInternal_NumRange *r3, const OScInternal_NumRange *r4) {
    OScInternal_NumRange *tmp = OScInternal_NumRange_Intersection3(r1, r2, r3);
    OScInternal_NumRange *ret = OScInternal_NumRange_Intersection(tmp, r4);
    OScInternal_NumRange_Destroy(tmp);
    return ret;
}

OScInternal_NumRange *OScInternal_NumRange_Intersection5(
    const OScInternal_NumRange *r1, const OScInternal_NumRange *r2,
    const OScInternal_NumRange *r3, const OScInternal_NumRange *r4,
    const OScInternal_NumRange *r5) {
    OScInternal_NumRange *tmp =
        OScInternal_NumRange_Intersection4(r1, r2, r3, r4);
    OScInternal_NumRange *ret = OScInternal_NumRange_Intersection(tmp, r5);
    OScInternal_NumRange_Destroy(tmp);
    return ret;
}

OScInternal_NumRange *OScInternal_NumRange_Intersection6(
    const OScInternal_NumRange *r1, const OScInternal_NumRange *r2,
    const OScInternal_NumRange *r3, const OScInternal_NumRange *r4,
    const OScInternal_NumRange *r5, const OScInternal_NumRange *r6) {
    OScInternal_NumRange *tmp =
        OScInternal_NumRange_Intersection5(r1, r2, r3, r4, r5);
    OScInternal_NumRange *ret = OScInternal_NumRange_Intersection(tmp, r6);
    OScInternal_NumRange_Destroy(tmp);
    return ret;
}
