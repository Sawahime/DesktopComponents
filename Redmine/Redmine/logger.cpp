#include "framework.h"
#include <iostream>


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

	int width = 300;

	// 获取父窗口位置和尺寸
	RECT parentRect;
	GetWindowRect(hParent, &parentRect);

	// 计算日志窗口位置（紧挨着主窗口左侧）
	int x = parentRect.left - width;
	int y = parentRect.top;
	int height = parentRect.bottom - parentRect.top;

	m_hWnd = CreateWindowW(
		L"LogWindowClass", L"日志窗口",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		x, y, width, height,
		hParent, nullptr, GetModuleHandle(NULL), nullptr
	);
}


LRESULT CALLBACK Logger::LogWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE:
		std::cout << __FUNCTION__": " << "WM_CREATE" << std::endl;
		break;
	case WM_SIZE:
		std::cout << __FUNCTION__": " << "WM_SIZE" << std::endl;
		break;
	case WM_COMMAND:
		std::cout << __FUNCTION__": " << "WM_COMMAND" << std::endl;
		break;
	case WM_CLOSE:
		std::cout << __FUNCTION__": " << "WM_CLOSE" << std::endl;
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}