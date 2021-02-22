#pragma once

#include "OpenScanLibPrivate.h"

// Internal errors
OSc_RichError *OScInternal_Error_WrongConstraintType()
{
    return OScInternal_Error_Create("Wrong constraint type");
}
