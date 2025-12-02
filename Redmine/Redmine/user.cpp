#include "framework.h"
#include "user.h"
#include "main.h"


bool RedmineUser::CreateUserWindow(HWND hParent) {
	m_hInstance = GetModuleHandle(NULL);

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = m_hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = m_WindowClass;
	ATOM atom = RegisterClassEx(&wc);
	if (atom == 0) {
		return false;
	}

	int windowWidth = 400, windowHeight = 300;
	int x = (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2;
	int y = (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2;
	m_hWnd = CreateWindowW(
		m_WindowClass, m_WindowTitle, WS_OVERLAPPEDWINDOW,
		x, y, windowWidth, windowHeight,
		hParent, nullptr, m_hInstance, this
	);
	if (!m_hWnd) {
		return false;
	}

	int labelX = 20, labelWidth = 100, labelHeight = 25;
	int editX = 130, editWidth = 240, editHeight = 25;
	int btnX = 20, btnWidth = 60, btnHeight = 30;
	int currentY = 20;
	int verticalSpacing = 40;

	m_hFirstNameLabel = CreateWindowW(
		L"STATIC", L"First name:", WS_CHILD | WS_VISIBLE,
		labelX, currentY, labelWidth, labelHeight,
		m_hWnd, nullptr, m_hInstance, nullptr
	);
	m_hFirstNameEdit = CreateWindowW(
		L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		editX, currentY, editWidth, editHeight,
		m_hWnd, nullptr, m_hInstance, nullptr
	);
	currentY += verticalSpacing;

	m_hLastNameLabel = CreateWindowW(
		L"STATIC", L"Last name:", WS_CHILD | WS_VISIBLE,
		labelX, currentY, labelWidth, labelHeight,
		m_hWnd, nullptr, m_hInstance, nullptr
	);
	m_hLastNameEdit = CreateWindowW(
		L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		editX, currentY, editWidth, editHeight,
		m_hWnd, nullptr, m_hInstance, nullptr
	);
	currentY += verticalSpacing;

	m_hApiKeyLabel = CreateWindowW(
		L"STATIC", L"ApiKey:", WS_CHILD | WS_VISIBLE,
		labelX, currentY, labelWidth, labelHeight,
		m_hWnd, nullptr, m_hInstance, nullptr
	);
	m_hApiKeyEdit = CreateWindowW(
		L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		editX, currentY, editWidth, editHeight,
		m_hWnd, nullptr, m_hInstance, nullptr
	);
	currentY += verticalSpacing;

	m_hBtnSave = CreateWindowW(
		L"BUTTON", L"Save",
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		btnX, currentY, btnWidth, btnHeight,
		m_hWnd, (HMENU)m_hMenuBtnSave, GetModuleHandle(NULL), nullptr
	);
	currentY += verticalSpacing;

	return true;
}


LRESULT RedmineUser::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static RedmineUser* user = nullptr;
	switch (message) {
	case WM_CREATE:
	{
		LPCREATESTRUCT lpCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		user = static_cast<RedmineUser*>(lpCreateStruct->lpCreateParams);
	}
	break;
	case WM_DESTROY:
		break;
	case WM_CLOSE:
		if (user) user->HideUserWindow();
		break;
	case WM_COMMAND:
	{
		WORD cmd = LOWORD(wParam);
		switch (cmd) {
		case m_hMenuBtnSave:
		{
			if (user) {
				int len;

				len = GetWindowTextLength(user->m_hFirstNameEdit);
				if (len > 0) {
					user->m_firstName.resize(len);
					GetWindowText(user->m_hFirstNameEdit, &user->m_firstName[0], len + 1);
				}
				else {
					user->m_firstName.clear();
				}

				len = GetWindowTextLength(user->m_hLastNameEdit);
				if (len > 0) {
					user->m_lastName.resize(len);
					GetWindowText(user->m_hLastNameEdit, &user->m_lastName[0], len + 1);
				}
				else {
					user->m_lastName.clear();
				}

				len = GetWindowTextLength(user->m_hApiKeyEdit);
				if (len > 0) {
					user->m_apiKey.resize(len);
					GetWindowText(user->m_hApiKeyEdit, &user->m_apiKey[0], len + 1);
				}
				else {
					user->m_apiKey.clear();
				}

				std::cout << "First Name: " << WStringToString(user->m_firstName) << std::endl;
				std::cout << "Last Name: " << WStringToString(user->m_lastName) << std::endl;
				std::cout << "ApiKey: " << WStringToString(user->m_apiKey) << std::endl;
			}
		}
		break;
		}
	}
	break;
	case WM_CTLCOLORSTATIC:
	{
		HDC hdcStatic = (HDC)wParam;
		if ((HWND)lParam == user->m_hFirstNameLabel ||
			(HWND)lParam == user->m_hLastNameLabel ||
			(HWND)lParam == user->m_hApiKeyLabel)
		{
			SetBkMode(hdcStatic, TRANSPARENT);
			return (LRESULT)GetStockObject(NULL_BRUSH);
		}
	}
	break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


void RedmineUser::ShowUserWindow() const {
	ShowWindow(m_hWnd, SW_SHOW);
}

void RedmineUser::HideUserWindow() const {
	ShowWindow(m_hWnd, SW_HIDE);
}