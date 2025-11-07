#pragma once

class RedmineIssuesWidget {
public:
	void InitWindowRectArea(HWND hWnd);
	void Draw(HDC hdc);
	void DrawTitle(HDC hdc);
	void DrawIssuesList(HDC hdc);

	void RequestIssues();

private:
	std::string ParsePyDictValueByKey(PyObject* dict, const char* key);

private:
	RECT m_TitleRect = { 0 };
	RECT m_IssuesRect = { 0 };
	LPCWSTR m_TitleText = L"Redmine Issues";
};