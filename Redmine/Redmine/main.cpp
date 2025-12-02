#include "framework.h"
#include "main.h"
#include "logger.h"
#include "user.h"
#include "redmine.h"


RedmineIssuesWidget* g_redmine = nullptr;


int APIENTRY wWinMain(
	_In_ HINSTANCE		hInstance,
	_In_opt_ HINSTANCE	hPrevInstance,
	_In_ LPWSTR			lpCmdLine,
	_In_ int			nCmdShow
) {
	FunctionEntryLog;
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	switch (SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE)) {
	case S_OK: // The DPI awareness for the app was set successfully
		break;
	case E_INVALIDARG: // The value passed in is not valid.
	case E_ACCESSDENIED: // The DPI awareness is already set, either by calling this API previously or through the application (.exe) manifest.
		return false;
	}

	g_redmine = new RedmineIssuesWidget(hInstance);
	g_redmine->RegisterWindowClass();
	if (!g_redmine->InitInstance(nCmdShow)) {
		return FALSE;
	}

	const std::wstring userFile = L"userdata.dat";

	// 示例：保存用户信息
	//if (g_redmine->SaveUserInfo(userFile, "Louis", "5f46eaf59a601436e657869dfadf68cf416e8602")) {
	//	MessageBox(NULL, L"用户信息保存成功", L"成功", MB_OK);
	//}

	// 示例：读取用户信息
	//std::string username, password;
	//if (g_redmine->LoadUserInfo(userFile, username, password)) {
	//	std::wstring msg = L"读取到的用户信息:\n用户名: " + std::wstring(username.begin(), username.end()) + L"\n密码: " + std::wstring(password.begin(), password.end());
	//	MessageBox(NULL, msg.c_str(), L"用户信息", MB_OK);
	//}


	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_REDMINE));
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	delete g_redmine;
	FunctionExitLog;
	return (int)msg.wParam;
}


std::wstring StringToWString(const std::string& str) {
	if (str.empty()) return std::wstring();

	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
	if (len == 0) return std::wstring();

	std::wstring wstr(len - 1, 0);
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &wstr[0], len);
	return wstr;
}

std::string WStringToString(const std::wstring& wstr) {
	if (wstr.empty()) return std::string();

	int requiredSize = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
	if (requiredSize == 0) {
		return std::string();
	}

	std::string result(requiredSize, 0);
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), static_cast<int>(wstr.size()), &result[0], requiredSize, nullptr, nullptr);

	return result;
}

std::string WCharToString(const wchar_t* wstr) {
	if (wstr == nullptr) {
		return "";
	}

	int bufferSize = WideCharToMultiByte(CP_ACP, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
	if (bufferSize == 0) {
		return "";
	}

	std::vector<char> buffer(bufferSize);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, buffer.data(), bufferSize, nullptr, nullptr);

	return std::string(buffer.data());
}
