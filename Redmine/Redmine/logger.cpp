#include "framework.h"


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


LRESULT CALLBACK Logger::LogWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static Logger* logger = nullptr;

	switch (message) {
	case WM_CREATE: {
		std::cout << __FUNCTION__": " << "WM_CREATE" << std::endl;
		LPCREATESTRUCT lpCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		logger = static_cast<Logger*>(lpCreateStruct->lpCreateParams);

		if (logger) logger->ShowLogWindow();
		break;
	}
	case WM_SIZE:
		std::cout << __FUNCTION__": " << "WM_SIZE" << std::endl;
		break;
	case WM_COMMAND:
		if (logger) logger->EvtCommand(hWnd, message, wParam, lParam);
		break;
	case WM_CLOSE:
		std::cout << __FUNCTION__": " << "WM_CLOSE" << std::endl;
		if (logger) logger->HideLogWindow();
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT Logger::EvtCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) const {
	WORD cmd = LOWORD(wParam);
	switch (cmd) {
	case IDC_BTN_CLEAR:
		break;
	case IDC_BTN_COPY:
		break;
	case IDC_BTN_SAVE:
		break;
	}

	return 0;
}