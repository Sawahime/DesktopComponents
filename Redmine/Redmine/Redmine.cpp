#include "framework.h"
#include "logger.h"
#include "user.h"
#include "redmine.h"
#include "main.h"


extern RedmineIssuesWidget* g_redmine;


void RedmineIssuesWidget::InitMessageFunctionTable() {
#define DefineMsgFunc(message, func) m_MessageTable[message] = [this](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) { return this->func(hwnd, msg, wparam, lparam); };
	DefineMsgFunc(WM_CREATE, EvtCreateWindow);
	DefineMsgFunc(WM_DESTROY, EvtDestroyWindow);		// 发送退出消息并返回
	DefineMsgFunc(WM_PAINT, EvtPaint);				// 绘制主窗口
	DefineMsgFunc(WM_COMMAND, EvtCommand);			// 处理应用程序菜单
	DefineMsgFunc(WM_TIMER, EvtTimer);
	DefineMsgFunc(WM_TRAYICON, EvtTrayNotify);
}

ATOM RedmineIssuesWidget::RegisterWindowClass() const {
	WNDCLASSEXW wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = m_hInstance;
	wcex.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_REDMINE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_REDMINE);
	wcex.lpszClassName = m_szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

bool RedmineIssuesWidget::InitInstance(int nCmdShow) {
	int screenWidth = GetSystemMetrics(SM_CXFULLSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYFULLSCREEN);
	int windowWidth = screenWidth / 4;
	int windowHeight = screenHeight;
	int xPos = screenWidth - windowWidth;
	int yPos = 0;

	m_hWnd = CreateWindowW(
		m_szWindowClass, m_szTitle, WS_OVERLAPPEDWINDOW,
		xPos, yPos, windowWidth, windowHeight,
		nullptr, nullptr, m_hInstance, this
	);
	if (!m_hWnd) {
		return false;
	}

	SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);

	// Set the layered window style to support the adjustment of opacity
	SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(m_hWnd, 0, m_Opacity, LWA_ALPHA);

	// Do not show the window in task bar
	SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);

	// hook mouse message for scrolling screen
	//m_hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, GetModuleHandle(NULL), 0);

	m_Logger->CreateLogWindow(m_hWnd);
	m_User->CreateUserWindow(m_hWnd);

	RequestIssues();
	SetTimer(m_hWnd, m_RequestTimerId, m_RequestTimerIntervalMs, nullptr);

	ShowWindow(m_hWnd, nCmdShow);
	UpdateWindow(m_hWnd);

	return true;
}


void RedmineIssuesWidget::InitWindowRectArea(HWND hWnd) {
	GetClientRect(hWnd, &m_TitleRect);
	m_TitleRect.bottom /= 6;

	GetClientRect(hWnd, &m_IssuesRect);
	m_IssuesRect.top = m_TitleRect.bottom;
}

void RedmineIssuesWidget::Draw(HDC hdc) {
	DrawTitle(hdc);
	DrawIssuesList(hdc);
}

void RedmineIssuesWidget::DrawTitle(HDC hdc) {
	// Title
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(0, 0, 0));
	HFONT hFont = CreateFontW(
		36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI"
	);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
	DrawTextW(hdc, m_TitleText, -1, &m_TitleRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);
}

void RedmineIssuesWidget::DrawIssuesList(HDC hdc) {
	// Set the background of the issues area
	HBRUSH hBackgroundBrush = CreateSolidBrush(RGB(87, 192, 252));
	FillRect(hdc, &m_IssuesRect, hBackgroundBrush);
	DeleteObject(hBackgroundBrush);

	if (m_Issues.empty()) return;

	// Calculate the size and spacing of the cards
	int margins = 10; // Left and right margins (in px).
	int cardWidth = m_IssuesRect.right - m_IssuesRect.left - margins * 2;
	int cardHeight = 72;
	int cardSpacing = 10; // The spacing between the cards
	int startY = m_IssuesRect.top + 10 + m_ContentStartYOffset;

	m_TotalContentHeight = (int)m_Issues.size() * (cardHeight + cardSpacing);

	for (size_t i = 0; i < m_Issues.size(); i++) {
		// Calculate the start y-coordinate of each card
		int cardY = startY + (int)i * (cardHeight + cardSpacing);

		// Check if the card is within the visible area
		if (cardY + cardHeight < m_IssuesRect.top || cardY > m_IssuesRect.bottom) {
			continue; // Not within the visible area, skip the drawing.
		}

		// Define the rectangular area of the card
		RECT cardRect = {
			m_IssuesRect.left + margins,// left
			cardY,// top
			m_IssuesRect.left + margins + cardWidth,// right
			cardY + cardHeight// bottom
		};

		DrawSingleIssueCard(hdc, m_Issues[i], cardRect);
	}
}

void RedmineIssuesWidget::DrawSingleIssueCard(HDC hdc, const json& issue, RECT& cardRect) {
	// Draw the background of the card
	HBRUSH hCardBrush = CreateSolidBrush(RGB(255, 255, 255));
	HPEN hBorderPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hCardBrush);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hBorderPen);
	RoundRect(hdc, cardRect.left, cardRect.top, cardRect.right, cardRect.bottom, 12, 12);
	SelectObject(hdc, hOldBrush);
	SelectObject(hdc, hOldPen);
	DeleteObject(hCardBrush);
	DeleteObject(hBorderPen);

	// Set text properties
	SetBkMode(hdc, TRANSPARENT);

	// Create font
	HFONT hBoldFont = CreateFontW(
		20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei UI"
	);
	HFONT hNormalFont = CreateFontW(
		14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei UI"
	);
	HFONT hSmallFont = CreateFontW(
		12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei UI"
	);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hBoldFont);

	// Draw the ID and subject
	SetTextColor(hdc, RGB(70, 130, 180));

	std::string id_str = JsonToString(issue["id"]);
	std::string subject = JsonToString(issue["subject"]);
	std::wstring subjectText = L"#" + StringToWString(id_str) + L" " + StringToWString(EncodingConverter::utf8_to_local(subject));

	RECT subjectRect = { cardRect.left + 15, cardRect.top + 12, cardRect.right - 15, cardRect.top + 35 };
	DrawTextW(hdc, subjectText.c_str(), -1, &subjectRect, DT_LEFT | DT_SINGLELINE);

	// 绘制进度条
	DrawProgressBar(hdc, issue, cardRect);

	SelectObject(hdc, hOldFont);
	DeleteObject(hBoldFont);
	DeleteObject(hNormalFont);
	DeleteObject(hSmallFont);
}

void RedmineIssuesWidget::DrawProgressBar(HDC hdc, const json& issue, RECT& cardRect) {
	int actualProgress = 0;
	std::string done_ratio = JsonToString(issue["done_ratio"]);

	try {
		if (!done_ratio.empty() && done_ratio != "None") {
			actualProgress = std::stoi(done_ratio);
		}
	}
	catch (const std::exception&) {
		actualProgress = 0;
	}

	// 进度条位置和尺寸
	int margins = 15;
	int barWidth = cardRect.right - cardRect.left - margins * 2 - 48;
	int barHeight = 16;
	int barX = cardRect.left + margins;
	int barY = cardRect.bottom - barHeight - 12;

	// 绘制进度条背景
	HBRUSH hBgBrush = CreateSolidBrush(RGB(240, 240, 240));
	HPEN hBorderPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBgBrush);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hBorderPen);
	Rectangle(hdc, barX, barY, barX + barWidth, barY + barHeight);

	// 获取开始日期和截止日期
	std::string start_date = JsonToString(issue["start_date"]);
	std::string due_date = JsonToString(issue["due_date"]);

	if (!start_date.empty() && start_date != "None" &&
		!due_date.empty() && due_date != "None")
	{
		SYSTEMTIME startDate = { 0 };
		sscanf_s(start_date.c_str(), "%hu-%hu-%hu", &startDate.wYear, &startDate.wMonth, &startDate.wDay);

		SYSTEMTIME dueDate = { 0 };
		sscanf_s(due_date.c_str(), "%hu-%hu-%hu", &dueDate.wYear, &dueDate.wMonth, &dueDate.wDay);

		SYSTEMTIME currentDate;
		GetLocalTime(&currentDate);

		// 将 SYSTEMTIME 转换为 FILETIME 以便计算
		FILETIME ftStart, ftDue, ftCurrent;
		SystemTimeToFileTime(&startDate, &ftStart);
		SystemTimeToFileTime(&dueDate, &ftDue);
		SystemTimeToFileTime(&currentDate, &ftCurrent);

		// 将 FILETIME 转换为 ULARGE_INTEGER 进行数值计算
		ULARGE_INTEGER ullStart = { 0 };
		ullStart.LowPart = ftStart.dwLowDateTime;
		ullStart.HighPart = ftStart.dwHighDateTime;

		ULARGE_INTEGER ullDue = { 0 };
		ullDue.LowPart = ftDue.dwLowDateTime;
		ullDue.HighPart = ftDue.dwHighDateTime;

		ULARGE_INTEGER ullCurrent = { 0 };
		ullCurrent.LowPart = ftCurrent.dwLowDateTime;
		ullCurrent.HighPart = ftCurrent.dwHighDateTime;

		int theoreticalProgress;

		if (ullCurrent.QuadPart <= ullStart.QuadPart || ullDue.QuadPart <= ullStart.QuadPart) {
			theoreticalProgress = 0;
		}
		else {
			theoreticalProgress = (ullCurrent.QuadPart - ullStart.QuadPart) * 100 / (ullDue.QuadPart - ullStart.QuadPart);
			theoreticalProgress = min(theoreticalProgress, 100);
		}

		// 绘制理论进度（红色）
		HBRUSH hTheoreticalBrush = CreateSolidBrush(RGB(255, 100, 100));
		SelectObject(hdc, hTheoreticalBrush);
		Rectangle(hdc, barX, barY, barX + barWidth * theoreticalProgress / 100, barY + barHeight);
		DeleteObject(hTheoreticalBrush);
	}

	// 绘制实际进度（绿色）
	int actualWidth = (barWidth * actualProgress) / 100;
	HBRUSH hActualBrush = CreateSolidBrush(RGB(50, 205, 50));
	SelectObject(hdc, hActualBrush);
	Rectangle(hdc, barX, barY, barX + actualWidth, barY + barHeight);

	// 绘制进度文本
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(100, 100, 100));
	HFONT hSmallFont = CreateFontW(
		barHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei UI"
	);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hSmallFont);
	std::wstring progressText = std::to_wstring(actualProgress) + L"% 完成";
	RECT textRect = { barX + barWidth + 10, barY - 2, barX + barWidth + 150, barY + barHeight + 2 };
	DrawTextW(hdc, progressText.c_str(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	SelectObject(hdc, hOldBrush);
	SelectObject(hdc, hOldPen);
	SelectObject(hdc, hOldFont);
	DeleteObject(hActualBrush);
	DeleteObject(hBgBrush);
	DeleteObject(hBorderPen);
	DeleteObject(hSmallFont);
}


LRESULT RedmineIssuesWidget::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (auto it = g_redmine->m_MessageTable.find(message); it != g_redmine->m_MessageTable.end()) return it->second(hWnd, message, wParam, lParam);
	else return DefWindowProc(hWnd, message, wParam, lParam);
}


LRESULT RedmineIssuesWidget::MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0) {
		if (wParam == WM_MOUSEWHEEL) {
			MSLLHOOKSTRUCT* p = (MSLLHOOKSTRUCT*)lParam;

			RECT rect;
			GetWindowRect(g_redmine->m_hWnd, &rect);

			if (PtInRect(&rect, p->pt)) {
				SHORT delta = HIWORD(p->mouseData);// up=120, down=-120
				g_redmine->HandleMouseWheel(delta);
			}
		}
	}

	return CallNextHookEx(g_redmine->m_hMouseHook, nCode, wParam, lParam);
}


INT_PTR RedmineIssuesWidget::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	switch (message) {
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


LRESULT RedmineIssuesWidget::EvtCreateWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	g_redmine->InitNotifyIconData(hWnd);
	return 0;
}


LRESULT RedmineIssuesWidget::EvtCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId = LOWORD(wParam);
	switch (wmId) {
	case IDM_LOGIN:
		g_redmine->m_User->ShowUserWindow();
		break;
	case IDM_PREFERENCE:
		MessageBox(NULL, L"偏好功能尚未实现", L"成功", MB_OK);
		break;
	case IDM_SHOWLOG:
		g_redmine->m_Logger->ShowLogWindow();
		break;
	case IDM_ABOUT:
		DialogBox(g_redmine->m_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
		break;
	case IDM_EXIT:
		DestroyWindow(hWnd);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}


LRESULT RedmineIssuesWidget::EvtPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);// Handle to Device Context

	g_redmine->InitWindowRectArea(hWnd);
	g_redmine->Draw(hdc);

	EndPaint(hWnd, &ps);

	return 0;
}


LRESULT RedmineIssuesWidget::EvtTimer(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (wParam == m_RequestTimerId) {
		RequestIssues();
	}
	return 0;
}


LRESULT RedmineIssuesWidget::EvtTrayNotify(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) const {
	if (lParam == WM_RBUTTONUP) {// Right mouse button Up
		// Show menu
		POINT pt;
		GetCursorPos(&pt);
		SetForegroundWindow(hWnd);
		TrackPopupMenu(m_hTrayMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);// hold on and wait for click
		PostMessage(hWnd, WM_NULL, 0, 0);
	}
	else if (lParam == WM_LBUTTONDBLCLK) {// Left mouse button double-click
		if (IsWindowVisible(hWnd)) {
			ShowWindow(hWnd, SW_HIDE);
		}
		else {
			ShowWindow(hWnd, SW_SHOW);
			SetForegroundWindow(hWnd);
		}
	}

	return 0;
}


LRESULT RedmineIssuesWidget::EvtDestroyWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	std::cout << __FUNCTION__": " << "Entry" << std::endl;

	g_redmine->m_Logger->DeleteLogWindow();

	KillTimer(hWnd, g_redmine->m_RequestTimerId);
	Shell_NotifyIcon(NIM_DELETE, &g_redmine->m_NotifyIconData);

	PostQuitMessage(0);

	return 0;
}


void RedmineIssuesWidget::InitNotifyIconData(HWND hWnd) {
	if (!m_hTrayMenu) {
		m_hTrayMenu = CreatePopupMenu();

		/*
		 The following groups of flags cannot be used together :
			MF_BYCOMMAND and MF_BYPOSITION
			MF_DISABLED, MF_ENABLED, and MF_GRAYED
			MF_BITMAP, MF_STRING, MF_OWNERDRAW, and MF_SEPARATOR
			MF_MENUBARBREAK and MF_MENUBREAK
			MF_CHECKED and MF_UNCHECKED
		*/
		InsertMenu(m_hTrayMenu, -1, MF_BYPOSITION | MF_STRING, IDM_LOGIN, L"Login");
		InsertMenu(m_hTrayMenu, -1, MF_BYPOSITION | MF_STRING, IDM_PREFERENCE, L"Preference...");
		InsertMenu(m_hTrayMenu, -1, MF_BYPOSITION | MF_STRING, IDM_SHOWLOG, L"Show Logs");
		InsertMenu(m_hTrayMenu, -1, MF_BYPOSITION | MF_STRING, IDM_ABOUT, L"About");
		InsertMenu(m_hTrayMenu, -1, MF_BYPOSITION | MF_STRING, IDM_EXIT, L"Quit");
	}

	// Init system tray icon
	m_NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	m_NotifyIconData.hWnd = hWnd;
	m_NotifyIconData.uID = ID_TRAY_ICON;
	m_NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_NotifyIconData.uCallbackMessage = WM_TRAYICON;
	m_NotifyIconData.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_TRAY_ICON));
	wcscpy_s(m_NotifyIconData.szTip, L"Redmine Issues Widget");

	Shell_NotifyIcon(NIM_ADD, &m_NotifyIconData);
}

void RedmineIssuesWidget::DeInitNotifyIconData() {
	Shell_NotifyIcon(NIM_DELETE, &m_NotifyIconData);

	if (m_hTrayMenu) {
		DestroyMenu(m_hTrayMenu);
		m_hTrayMenu = nullptr;
	}
}

void RedmineIssuesWidget::HandleMouseWheel(int delta) {
	int scrollAmount = -delta / WHEEL_DELTA; // Change to scroll rows
	int offset = m_ContentStartYOffset - scrollAmount * 30; // Scroll 30 pixels each time

	// upperLimit <= start_y_offset <= lowerLimit
	int upperLimit = (m_IssuesRect.bottom - m_IssuesRect.top) - m_TotalContentHeight;
	int lowerLimit = 0;
	offset = max(upperLimit, offset);
	offset = min(lowerLimit, offset);

	if (offset != m_ContentStartYOffset) {
		m_ContentStartYOffset = offset;

		// Redraw
		if (m_hWnd) {
			InvalidateRect(m_hWnd, &m_IssuesRect, TRUE);// it will trigger case WM_PAINT to redraw
		}
	}
}


json RedmineIssuesWidget::HttpGetIssues(int limit = 100, int offset = 0) {
	try {
		httplib::Client cli(m_HostUrl, m_Port);

		httplib::Params params = {
			{"limit", std::to_string(limit)},
			{"offset", std::to_string(offset)}
		};

		httplib::Headers headers;
		std::string apikey = WStringToString(m_User->GetApiKey());
		if (!apikey.empty()) {
			headers = { {"X-Redmine-API-Key", apikey} };
		}

		auto res = cli.Get("/issues.json", params, headers);
		if (!res) {
			std::cerr << "请求错误: 无法连接到服务器" << std::endl;
			return nullptr;
		}
		if (res->status != 200) {
			std::cerr << "请求错误: HTTP " << res->status << std::endl;
			return nullptr;
		}

		return json::parse(res->body);
	}
	catch (const std::exception& e) {
		std::cerr << "请求错误: " << e.what() << std::endl;
		return nullptr;
	}
}

json RedmineIssuesWidget::HttpGetAllIssues(int limit = 100) {
	json all_issues = json::array();
	int offset = 0;

	while (true) {
		json ret = HttpGetIssues(limit, offset);
		if (ret.is_null()) {
			break;
		}

		if (!ret.contains("issues") || !ret["issues"].is_array()) {
			break;
		}

		json issues = ret["issues"];
		for (const auto& issue : issues) {
			all_issues.push_back(issue);
		}

		if (issues.size() < static_cast<size_t>(limit)) {
			break;
		}
		else {
			offset += issues.size();
		}
	}

	return all_issues;
}

json RedmineIssuesWidget::HttpGetAllIssues(std::string assignee_name) {
	json all_issues = json::array();
	int limit = 100, offset = 0;

	while (true) {
		json ret = HttpGetIssues(limit, offset);
		if (ret.is_null()) {
			break;
		}

		if (!ret.contains("issues") || !ret["issues"].is_array()) {
			break;
		}

		json issues = ret["issues"];
		for (const auto& issue : issues) {
			if (issue.contains("assigned_to") &&
				!issue["assigned_to"].is_null() &&
				issue["assigned_to"].contains("name") &&
				issue["assigned_to"]["name"].is_string() &&
				issue["assigned_to"]["name"] == assignee_name)
			{
				all_issues.push_back(issue);
			}
		}

		if (issues.size() < static_cast<size_t>(limit)) {
			break;
		}
		else {
			offset += issues.size();
		}
	}

	return all_issues;
}

void RedmineIssuesWidget::RequestIssues() {
	const std::wstring firstName = m_User->GetFirstName();
	const std::wstring lastName = m_User->GetLastName();

	m_Issues.clear();
	m_Issues = HttpGetAllIssues(EncodingConverter::local_to_utf8(WStringToString(firstName + L" " + lastName)));
	std::cout << "\nTotally get " << m_Issues.size() << " issue(s): " << std::endl;
	for (const auto& issue : m_Issues) {
		std::cout << issue["id"] << "    " << EncodingConverter::utf8_to_local(issue["subject"]) << std::endl;
	}
}
