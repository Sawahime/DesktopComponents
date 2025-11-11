#pragma once

#include "resource.h"

#define DebugPrint(message) \
    do { \
        std::wstringstream ss; \
        ss << __FUNCTION__ << message; \
        OutputDebugString(ss.str().c_str()); \
    } while(0)

#define FunctionEntryLog DebugPrint(L" Entry" << std::endl);
#define FunctionExitLog DebugPrint(L" Exit" << std::endl);