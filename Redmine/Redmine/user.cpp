#include "framework.h"
#include "user.h"
#include "main.h"


bool RedmineUser::CreateUserWindow(HWND hParent) {
	if (!LoadUserInfo()) {
		std::cout << __FUNCTION__": " << "LoadUserInfo failed" << std::endl;
	}

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
		L"EDIT", m_FirstName.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
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
		L"EDIT", m_LastName.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
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
		L"EDIT", m_ApiKey.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
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
		switch (LOWORD(wParam)) {
		case m_hMenuBtnSave:
			if (user) user->SaveUserInfo();
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


bool RedmineUser::SaveUserInfo() {
	int len;

	len = GetWindowTextLength(m_hFirstNameEdit);
	if (len > 0) {
		m_FirstName.resize(len);
		GetWindowText(m_hFirstNameEdit, &m_FirstName[0], len + 1);
	}
	else {
		m_FirstName.clear();
	}

	len = GetWindowTextLength(m_hLastNameEdit);
	if (len > 0) {
		m_LastName.resize(len);
		GetWindowText(m_hLastNameEdit, &m_LastName[0], len + 1);
	}
	else {
		m_LastName.clear();
	}

	len = GetWindowTextLength(m_hApiKeyEdit);
	if (len > 0) {
		m_ApiKey.resize(len);
		GetWindowText(m_hApiKeyEdit, &m_ApiKey[0], len + 1);
	}
	else {
		m_ApiKey.clear();
	}

	std::string firstName = WStringToString(m_FirstName);
	std::string lastName = WStringToString(m_LastName);
	std::string apiKey = WStringToString(m_ApiKey);

	std::cout << "First Name: " << firstName << std::endl;
	std::cout << "Last Name: " << lastName << std::endl;
	std::cout << "ApiKey: " << apiKey << std::endl;

	try {
		std::ofstream file("userdata.dat");
		if (!file) return false;

		file << firstName << '\n' << lastName << '\n' << apiKey << '\n';

		return file.good();
	}
	catch (...) {
		std::cout << __FUNCTION__": Unknown exception" << std::endl;
		return false;
	}
}

bool RedmineUser::LoadUserInfo() {
	std::ifstream file("userdata.dat");
	if (!file) return false;

	std::string firstName, lastName, apiKey;
	if (std::getline(file, firstName) &&
		std::getline(file, lastName) &&
		std::getline(file, apiKey)) {

		m_FirstName = StringToWString(firstName);
		m_LastName = StringToWString(lastName);
		m_ApiKey = StringToWString(apiKey);

		return true;
	}

	return false;
}