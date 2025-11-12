#include "framework.h"

#include <winhttp.h>
#include <iostream>
#pragma comment(lib, "winhttp.lib")
#include <shellapi.h>

#define MAX_LOADSTRING 100
constexpr UINT_PTR TIMER_ID_REDMINE = 1;
constexpr UINT TIMER_INTERVAL_MS = 30000;

HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

ATOM                RegisterWindowClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

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

	{
		// 分配控制台
		AllocConsole();

		FILE* f;
		freopen_s(&f, "CONOUT$", "w", stdout);

		std::cout << "Hello from Windows桌面程序!" << std::endl;
	}

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
	if (!hWnd)
	{
		return FALSE;
	}
	// 让窗口不在任务栏显示
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);

	RedmineIssuesWidget* redmine = new RedmineIssuesWidget();
	redmine->SetWindowHandle(hWnd);
	redmine->RequestIssues();
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)redmine);

	SetTimer(hWnd, TIMER_ID_REDMINE, TIMER_INTERVAL_MS, nullptr);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//FunctionEntryLog;
	RedmineIssuesWidget* redmine = (RedmineIssuesWidget*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	static NOTIFYICONDATA nid = {};

	switch (message)
	{
	case WM_COMMAND:
	{
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
			MessageBox(hWnd, L"显示日志窗口功能待实现", L"提示", MB_OK);
			//logger->CreateLogWindow(hWnd);
			break;
		case ID_TRAY_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		DebugPrint(L"message = WM_PAINT" << std::endl);
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
	}
	break;
	case WM_DESTROY:
	{
		DebugPrint(L"message = WM_DESTROY" << std::endl);

		Shell_NotifyIcon(NIM_DELETE, &nid);

		KillTimer(hWnd, TIMER_ID_REDMINE);
		if (redmine) {
			delete redmine;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
		}
		PostQuitMessage(0);
	}
	break;
	case WM_CREATE:
	{
		DebugPrint(L"message = WM_CREATE" << std::endl);

		// 初始化系统托盘图标
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = hWnd;
		nid.uID = ID_TRAY_ICON;
		nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nid.uCallbackMessage = WM_TRAYICON;
		nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_TRAY_ICON));
		wcscpy_s(nid.szTip, L"Redmine Issues Widget");

		Shell_NotifyIcon(NIM_ADD, &nid);
	}
	break;
	case WM_MOUSEWHEEL:
	{
		if (redmine) {
			int delta = GET_WHEEL_DELTA_WPARAM(wParam);// up=120, down=-120
			redmine->HandleMouseWheel(delta);
		}
	}
	break;
	case WM_TIMER:
	{
		DebugPrint(L"message = WM_TIMER" << std::endl);
		if (wParam == TIMER_ID_REDMINE && redmine) {
			redmine->RequestIssues();
		}
	}
	break;
	case WM_TRAYICON:
	{
		DebugPrint(L"message = WM_TRAYICON" << std::endl);
		if (lParam == WM_RBUTTONUP)
		{
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
		else if (lParam == WM_LBUTTONDBLCLK)
		{
			DebugPrint(L"	WM_LBUTTONDBLCLK" << std::endl);
			// 双击显示/隐藏主窗口
			if (IsWindowVisible(hWnd))
			{
				ShowWindow(hWnd, SW_HIDE);
			}
			else
			{
				ShowWindow(hWnd, SW_SHOW);
				SetForegroundWindow(hWnd);
			}
		}
	}
	break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
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
