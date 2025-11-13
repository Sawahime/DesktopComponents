#pragma once

#define DebugPrint(message) \
    do { \
        std::wstringstream ss; \
        ss << __FUNCTION__ <<": "<< message; \
        OutputDebugString(ss.str().c_str()); \
    } while(0)

#define FunctionEntryLog DebugPrint(L"Entry" << std::endl);
#define FunctionExitLog DebugPrint(L"Exit" << std::endl);

// 控件ID定义
#define IDC_EDIT_LOG          1001
#define IDC_STATIC_SPLITTER   1002
#define IDC_BTN_CLEAR         1003
#define IDC_BTN_COPY          1004
#define IDC_BTN_SAVE          1005

class Logger {
public:
	void CreateLogWindow(HWND hParent);

private:
	static LRESULT CALLBACK LogWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HWND m_hWnd;           // 主窗口句柄
	HWND m_hEditLog;       // 日志文本框句柄
	HWND m_hStaticSplitter;// 分割线句柄
	HWND m_hBtnClear;      // 清空按钮句柄
	HWND m_hBtnCopy;       // 复制按钮句柄
	HWND m_hBtnSave;       // 保存按钮句柄
};