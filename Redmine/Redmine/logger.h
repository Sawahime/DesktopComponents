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


class EditBoxStreamBuf : public std::streambuf {
public:
	EditBoxStreamBuf(HWND hEdit) : m_hEdit(hEdit) {}

protected:
	virtual int_type overflow(int_type c) override {
		if (c != traits_type::eof()) {
			m_buffer += static_cast<char>(c);
			if (c == '\n') {
				int len = GetWindowTextLengthA(m_hEdit);
				SendMessageA(m_hEdit, EM_SETSEL, len, len);
				SendMessageA(m_hEdit, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(m_buffer.c_str()));
				m_buffer.clear();
			}
		}
		return c;
	}

private:
	HWND m_hEdit = nullptr;
	std::string m_buffer;
};


class Logger {
public:
	void CreateLogWindow(HWND hParent);
	void DeleteLogWindow();

	void ShowLogWindow() const;
	void HideLogWindow() const;

private:
	void RedirectCout();
	void RestoreCout();

	static LRESULT CALLBACK LogWndProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT EvtCommand(HWND, UINT, WPARAM, LPARAM) const;

	void ClearLog() const;
	void CopyLog() const;
	void SaveLog() const;

private:
	HWND m_hWnd;           // 日志窗口句柄
	HWND m_hEditLog;       // 日志文本框句柄
	HWND m_hStaticSplitter;// 分割线句柄
	HWND m_hBtnClear;      // 清空按钮句柄
	HWND m_hBtnCopy;       // 复制按钮句柄
	HWND m_hBtnSave;       // 保存按钮句柄

	std::streambuf* m_EditBoxBuf;
	std::streambuf* m_OldCoutBuf;
};