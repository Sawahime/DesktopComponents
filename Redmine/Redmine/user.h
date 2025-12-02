#pragma once


class RedmineUser
{
public:
	bool CreateUserWindow(HWND hParent);
	void ShowUserWindow() const;
	void HideUserWindow() const;

public:// Setter&Getter
	const std::wstring& GetFirstName() const noexcept { return m_FirstName; }
	const std::wstring& GetLastName() const noexcept { return m_LastName; }
	const std::wstring& GetApiKey() const noexcept { return m_ApiKey; }

private:
	static const int m_hMenuBtnSave = 1;
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	bool SaveUserInfo();
	bool LoadUserInfo();

private:
	HINSTANCE m_hInstance = nullptr;

	LPCWSTR m_WindowClass = L"UserWindow";
	LPCWSTR m_WindowTitle = L"User";
	HWND m_hWnd = nullptr;

	HWND m_hFirstNameLabel = nullptr;
	HWND m_hFirstNameEdit = nullptr;
	std::wstring m_FirstName;

	HWND m_hLastNameLabel = nullptr;
	HWND m_hLastNameEdit = nullptr;
	std::wstring m_LastName;

	HWND m_hApiKeyLabel = nullptr;
	HWND m_hApiKeyEdit = nullptr;
	std::wstring m_ApiKey;

	HWND m_hBtnSave = nullptr;
};