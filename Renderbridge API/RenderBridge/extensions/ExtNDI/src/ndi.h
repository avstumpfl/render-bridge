#pragma once

// silence "__declspec attributes before linkage specification are ignored" warning 
#if defined(NDEBUG)
#  define PROCESSINGNDILIB_DEPRECATED
#endif

#include "Processing.NDI.Lib.h"
