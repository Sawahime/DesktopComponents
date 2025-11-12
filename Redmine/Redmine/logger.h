#pragma once

#define DebugPrint(message) \
    do { \
        std::wstringstream ss; \
        ss << __FUNCTION__ << message; \
        OutputDebugString(ss.str().c_str()); \
    } while(0)

#define FunctionEntryLog DebugPrint(L" Entry" << std::endl);
#define FunctionExitLog DebugPrint(L" Exit" << std::endl);

class Logger {
public:
	void CreateLogWindow(HWND hParent);

private:
	static LRESULT CALLBACK LogWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HWND m_hWnd;
};