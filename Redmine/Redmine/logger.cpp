#include "framework.h"
#include "logger.h"


void Logger::CreateLogWindow(HWND hParent) {
	if (!hParent) return;

	WNDCLASSEXW wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = LogWndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = L"LogWindowClass";
	RegisterClassExW(&wc);

	RECT parentRect;
	GetWindowRect(hParent, &parentRect);

	int width = 600;
	int height = parentRect.bottom - parentRect.top;
	int x = parentRect.left - width + 15;
	int y = parentRect.top;

	m_hWnd = CreateWindowW(
		L"LogWindowClass", L"日志窗口",
		WS_OVERLAPPEDWINDOW,
		x, y, width, height,
		hParent, nullptr, GetModuleHandle(NULL), this
	);
	if (!m_hWnd) return;

	RECT clientRect;
	GetClientRect(m_hWnd, &clientRect);
	int clientWidth = clientRect.right - clientRect.left;
	int clientHeight = clientRect.bottom - clientRect.top;

	int spacing = 10;// 纵向区域间隔
	int startY = 0;// It will increase as the number of controls increases.

#pragma region TextArea
	int textBoxHeight = clientHeight * 9 / 10;
	m_hEditLog = CreateWindowW(
		L"EDIT", L"",
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
		0, startY, clientWidth - 10, textBoxHeight,
		m_hWnd, (HMENU)IDC_EDIT_LOG, GetModuleHandle(NULL), nullptr
	);
	// 设置字体
	HFONT hFont = CreateFontW(
		16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Consolas"
	);
	if (hFont) {
		SendMessage(m_hEditLog, WM_SETFONT, (WPARAM)hFont, TRUE);
	}
	RedirectCout();
	startY += textBoxHeight + spacing;
#pragma endregion

#pragma region SplitterArea
	int splitterHeight = 2;
	m_hStaticSplitter = CreateWindowW(
		L"STATIC", L"",
		WS_VISIBLE | WS_CHILD | SS_ETCHEDHORZ,
		5, startY, clientWidth - 10, splitterHeight,
		m_hWnd, (HMENU)IDC_STATIC_SPLITTER, GetModuleHandle(NULL), nullptr
	);
	startY += splitterHeight + spacing;
#pragma endregion

#pragma region ButtonArea
	int buttonWidth = 100;
	int buttonHeight = clientHeight - startY - spacing;
	int buttonSpacing = 20;

	int totalButtonsWidth = 3 * buttonWidth + 2 * buttonSpacing;
	int startX = (clientWidth - totalButtonsWidth) / 2;

	m_hBtnClear = CreateWindowW(
		L"BUTTON", L"Clear",
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		startX, startY, buttonWidth, buttonHeight,
		m_hWnd, (HMENU)IDC_BTN_CLEAR, GetModuleHandle(NULL), nullptr
	);
	startX += buttonWidth + buttonSpacing;

	m_hBtnCopy = CreateWindowW(
		L"BUTTON", L"Copy",
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		startX, startY, buttonWidth, buttonHeight,
		m_hWnd, (HMENU)IDC_BTN_COPY, GetModuleHandle(NULL), nullptr
	);
	startX += buttonWidth + buttonSpacing;

	m_hBtnSave = CreateWindowW(
		L"BUTTON", L"Save",
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		startX, startY, buttonWidth, buttonHeight,
		m_hWnd, (HMENU)IDC_BTN_SAVE, GetModuleHandle(NULL), nullptr
	);

	// 设置按钮字体
	HFONT hButtonFont = CreateFontW(
		24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"微软雅黑"
	);
	if (hButtonFont) {
		SendMessage(m_hBtnClear, WM_SETFONT, (WPARAM)hButtonFont, TRUE);
		SendMessage(m_hBtnCopy, WM_SETFONT, (WPARAM)hButtonFont, TRUE);
		SendMessage(m_hBtnSave, WM_SETFONT, (WPARAM)hButtonFont, TRUE);
	}
#pragma endregion
}

void Logger::DeleteLogWindow() {
	RestoreCout();
}


void Logger::ShowLogWindow() const {
	ShowWindow(m_hWnd, SW_SHOW);
	//UpdateWindow(m_hWnd);
}

void Logger::HideLogWindow() const {
	ShowWindow(m_hWnd, SW_HIDE);
}


void Logger::RedirectCout() {
	if (!m_hEditLog) {
		return;
	}

	m_EditBoxBuf = new EditBoxStreamBuf(m_hEditLog);
	m_OldCoutBuf = std::cout.rdbuf(m_EditBoxBuf);
}

void Logger::RestoreCout() {
	if (m_OldCoutBuf) std::cout.rdbuf(m_OldCoutBuf);
	if (m_EditBoxBuf) delete m_EditBoxBuf;
}


LRESULT Logger::LogWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static Logger* logger = nullptr;

	switch (message) {
	case WM_CREATE: {
		LPCREATESTRUCT lpCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		logger = static_cast<Logger*>(lpCreateStruct->lpCreateParams);
		break;
	}
	case WM_SIZE:
		break;
	case WM_COMMAND:
		if (logger) logger->WndProcCommand(hWnd, message, wParam, lParam);
		break;
	case WM_CLOSE:
		if (logger) logger->HideLogWindow();
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT Logger::WndProcCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) const {
	WORD cmd = LOWORD(wParam);
	switch (cmd) {
	case IDC_BTN_CLEAR:
		ClearLog();
		break;
	case IDC_BTN_COPY:
		CopyLog();
		break;
	case IDC_BTN_SAVE:
		SaveLog();
		break;
	}

	return 0;
}


void Logger::ClearLog() const {
	SetWindowTextA(m_hEditLog, "");
}

void Logger::CopyLog() const {
	if (OpenClipboard(nullptr)) {
		EmptyClipboard();

		int textLength = GetWindowTextLengthA(m_hEditLog);
		if (textLength > 0) {
			HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, textLength + 1);
			if (hGlobal) {
				char* pGlobal = static_cast<char*>(GlobalLock(hGlobal));// lock the memory and return a pointer
				if (pGlobal) GetWindowTextA(m_hEditLog, pGlobal, textLength + 1);
				GlobalUnlock(hGlobal);

				SetClipboardData(CF_TEXT, hGlobal);
				std::cout << "Log content copied to clipboard" << std::endl;
			}
			else {
				std::cout << "Failed to allocate memory for clipboard" << std::endl;
			}
		}
		else {
			std::cout << "No content to copy" << std::endl;
		}

		CloseClipboard();
	}
	else {
		std::cout << "Failed to open clipboard" << std::endl;
	}
}

void Logger::SaveLog() const {
	if (!m_hEditLog) {
		std::cout << "No log window available" << std::endl;
		return;
	}

	// 获取编辑框文本长度
	int textLength = GetWindowTextLengthA(m_hEditLog);
	if (textLength <= 0) {
		std::cout << "No content to save" << std::endl;
		return;
	}

	// 分配缓冲区
	std::vector<char> buffer(textLength + 1);
	GetWindowTextA(m_hEditLog, buffer.data(), buffer.size());

	// 生成文件名（带时间戳）
	auto now = std::chrono::system_clock::now();
	auto time_t = std::chrono::system_clock::to_time_t(now);
	std::tm tm;
	localtime_s(&tm, &time_t);

	char filename[256];
	snprintf(filename, sizeof(filename),
		"log_%04d%02d%02d_%02d%02d%02d.txt",
		tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec);

	// 获取可执行文件目录
	char exePath[MAX_PATH];
	GetModuleFileNameA(nullptr, exePath, MAX_PATH);

	// 提取目录路径
	std::string directory = exePath;
	size_t lastSlash = directory.find_last_of("\\/");
	if (lastSlash != std::string::npos) {
		directory = directory.substr(0, lastSlash + 1);
	}

	// 完整文件路径
	std::string fullPath = directory + filename;

	// 保存文件
	std::ofstream file(fullPath, std::ios::out | std::ios::binary);
	if (file.is_open()) {
		file.write(buffer.data(), textLength);
		file.close();
		std::cout << "Log saved to: " << fullPath << std::endl;
	}
	else {
		std::cout << "Failed to save log to: " << fullPath << std::endl;
	}
}