#pragma once

#include "resource.h"

#define DebugPrint(message) \
    do { \
        std::wstringstream ss; \
        ss << L"[DEBUG] " << message; \
        OutputDebugString(ss.str().c_str()); \
    } while(0)

#define FunctionEntryLog DebugPrint(__FUNCTION__ << L" Entry" << std::endl);

class RedmineIssuesWidget {
public:
	void InitWindowRectArea(HWND hWnd);
	void Draw(HDC hdc);

private:
	RECT m_TitleRect = { 0 };
	RECT m_IssuesRect = { 0 };
	LPCWSTR m_HeadLineText = L"Redmine Issues";
};