#pragma once

class RedmineIssuesWidget {
public:
	void InitWindowRectArea(HWND hWnd);
	void Draw(HDC hdc);

private:
	RECT m_TitleRect = { 0 };
	RECT m_IssuesRect = { 0 };
	LPCWSTR m_TitleText = L"Redmine Issues";
};