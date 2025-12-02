#pragma once


class RedmineUser
{
public:
	bool CreateUserWindow(HWND hParent);
	void ShowUserWindow() const;
	void HideUserWindow() const;

private:
	static const int m_hMenuBtnSave = 1;
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

private:
	HINSTANCE m_hInstance = nullptr;

	LPCWSTR m_WindowClass = L"UserWindow";
	LPCWSTR m_WindowTitle = L"User";
	HWND m_hWnd = nullptr;

	HWND m_hFirstNameLabel = nullptr;
	HWND m_hFirstNameEdit = nullptr;
	std::wstring m_firstName;

	HWND m_hLastNameLabel = nullptr;
	HWND m_hLastNameEdit = nullptr;
	std::wstring m_lastName;

	HWND m_hApiKeyLabel = nullptr;
	HWND m_hApiKeyEdit = nullptr;
	std::wstring m_apiKey;

	HWND m_hBtnSave = nullptr;
};