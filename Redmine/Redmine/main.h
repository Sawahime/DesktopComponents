#pragma once

#include "resource.h"

#define DebugPrint(message) \
    do { \
        std::wstringstream ss; \
        ss << L"[DEBUG] " << message; \
        OutputDebugString(ss.str().c_str()); \
    } while(0)

#define FunctionEntryLog DebugPrint(__FUNCTION__ << L" Entry" << std::endl);