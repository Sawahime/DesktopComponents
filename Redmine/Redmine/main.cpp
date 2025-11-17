#include "framework.h"
#include <functional>

#define MAX_LOADSTRING 100
constexpr UINT_PTR TIMER_ID_REDMINE = 1;
constexpr UINT TIMER_INTERVAL_MS = 30000;

static LRESULT EvtCreateWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT EvtCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT EvtMouseWheel(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT EvtPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT EvtTimer(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT EvtTrayNotify(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT EvtDestroyWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void InitNotifyIconData(HWND hWnd);
static void DeInitNotifyIconData();
ATOM                RegisterWindowClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
NOTIFYICONDATA g_NotifyIconData;
static std::unordered_map<UINT, std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>> messageTable = {
	{WM_CREATE,			EvtCreateWindow},
	{WM_DESTROY,		EvtDestroyWindow},	// 发送退出消息并返回
	{WM_PAINT,			EvtPaint},			// 绘制主窗口
	{WM_MOUSEWHEEL,		EvtMouseWheel},
	{WM_COMMAND,		EvtCommand},		// 处理应用程序菜单
	{WM_TIMER,			EvtTimer},
	{WM_TRAYICON,		EvtTrayNotify},
};


int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	FunctionEntryLog;
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此处放置代码。

	// 初始化全局字符串
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_REDMINE, szWindowClass, MAX_LOADSTRING);
	RegisterWindowClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_REDMINE));

	MSG msg;

	// 主消息循环:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	FunctionExitLog;
	return (int)msg.wParam;
}


ATOM RegisterWindowClass(HINSTANCE hInstance) {
	FunctionEntryLog;

	WNDCLASSEXW wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_REDMINE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_REDMINE);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	FunctionEntryLog;

	hInst = hInstance; // 将实例句柄存储在全局变量中

	// 获取主屏幕尺寸
	int screenWidth = GetSystemMetrics(SM_CXFULLSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYFULLSCREEN);

	int windowWidth = screenWidth / 4;
	int windowHeight = screenHeight;

	int xPos = screenWidth - windowWidth;
	int yPos = 0;

	HWND hWnd = CreateWindowW(
		szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		xPos, yPos, windowWidth, windowHeight,
		nullptr, nullptr, hInstance, nullptr
	);
	if (!hWnd) {
		return FALSE;
	}
	// 让窗口不在任务栏显示
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);

	CManager* manager = new CManager();
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)manager);

	RedmineIssuesWidget* redmine = manager->GetRedmine();
	redmine->SetWindowHandle(hWnd);
	redmine->RequestIssues();

	Logger* logger = manager->GetLogger();
	logger->CreateLogWindow(hWnd);

	SetTimer(hWnd, TIMER_ID_REDMINE, TIMER_INTERVAL_MS, nullptr);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (auto it = messageTable.find(message); it != messageTable.end()) return it->second(hWnd, message, wParam, lParam);
	else return DefWindowProc(hWnd, message, wParam, lParam);
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


void InitNotifyIconData(HWND hWnd) {
	// Init system tray icon
	g_NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	g_NotifyIconData.hWnd = hWnd;
	g_NotifyIconData.uID = ID_TRAY_ICON;
	g_NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	g_NotifyIconData.uCallbackMessage = WM_TRAYICON;
	g_NotifyIconData.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_TRAY_ICON));
	wcscpy_s(g_NotifyIconData.szTip, L"Redmine Issues Widget");

	Shell_NotifyIcon(NIM_ADD, &g_NotifyIconData);
}

void DeInitNotifyIconData() {
	Shell_NotifyIcon(NIM_DELETE, &g_NotifyIconData);
}


LRESULT EvtCreateWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	InitNotifyIconData(hWnd);
	return 0;
}


LRESULT EvtCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	CManager* manager = (CManager*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	Logger* logger = nullptr;
	if (manager) {
		logger = manager->GetLogger();
	}

	int wmId = LOWORD(wParam);
	// 分析菜单选择:
	switch (wmId)
	{
	case IDM_ABOUT:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
		break;
	case IDM_EXIT:
		DestroyWindow(hWnd);
		break;
	case ID_TRAY_SHOW_LOG:
		if (logger) logger->ShowLogWindow();
		break;
	case ID_TRAY_EXIT:
		DestroyWindow(hWnd);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}


LRESULT EvtMouseWheel(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	CManager* manager = (CManager*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	RedmineIssuesWidget* redmine = nullptr;
	if (manager) redmine = manager->GetRedmine();

	if (redmine) {
		int delta = GET_WHEEL_DELTA_WPARAM(wParam);// up=120, down=-120
		redmine->HandleMouseWheel(delta);
	}

	return 0;
}


LRESULT EvtPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	DebugPrint(L"message = WM_PAINT" << std::endl);

	CManager* manager = (CManager*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	RedmineIssuesWidget* redmine = nullptr;
	if (manager) redmine = manager->GetRedmine();

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);// Handle to Device Context
	// 在此处添加使用 hdc 的任何绘图代码...
	{
		if (redmine) {
			redmine->InitWindowRectArea(hWnd);
			redmine->Draw(hdc);
		}
	}
	EndPaint(hWnd, &ps);

	return 0;
}


LRESULT EvtTimer(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	DebugPrint(L"message = WM_TIMER" << std::endl);

	CManager* manager = (CManager*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	RedmineIssuesWidget* redmine = nullptr;
	if (manager) redmine = manager->GetRedmine();

	if (wParam == TIMER_ID_REDMINE && redmine) {
		redmine->RequestIssues();
	}

	return 0;
}


LRESULT EvtTrayNotify(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	DebugPrint(L"message = WM_TRAYICON" << std::endl);

	if (lParam == WM_RBUTTONUP) {
		DebugPrint(L"	WM_RBUTTONUP" << std::endl);
		// 创建右键菜单
		HMENU hMenu = CreatePopupMenu();
		InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, ID_TRAY_SHOW_LOG, L"打开日志窗口");
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);  // 分隔线
		InsertMenu(hMenu, 2, MF_BYPOSITION | MF_STRING, ID_TRAY_EXIT, L"退出");

		// 显示菜单
		POINT pt;
		GetCursorPos(&pt);
		SetForegroundWindow(hWnd);
		TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
		PostMessage(hWnd, WM_NULL, 0, 0);
		DestroyMenu(hMenu);
	}
	else if (lParam == WM_LBUTTONDBLCLK) {
		DebugPrint(L"	WM_LBUTTONDBLCLK" << std::endl);
		// 双击显示/隐藏主窗口
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


LRESULT EvtDestroyWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	DebugPrint(L"message = WM_DESTROY" << std::endl);

	CManager* manager = (CManager*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	Logger* logger = nullptr;
	if (manager) logger = manager->GetLogger();

	if (logger) logger->DeleteLogWindow();

	KillTimer(hWnd, TIMER_ID_REDMINE);
	if (manager) {
		delete manager;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
	}

	Shell_NotifyIcon(NIM_DELETE, &g_NotifyIconData);
	PostQuitMessage(0);

	return 0;
}

