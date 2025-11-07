#pragma once

class RedmineIssuesWidget {
private:
	typedef struct _ISSUES_INFO {
		std::string id;
		std::string subject;
		std::string status;
		std::string priority;
		std::string done_ratio;
		std::string start_date;
		std::string due_date;
	}ISSUES_INFO, * PISSUES_INFO;

public:
	void InitWindowRectArea(HWND hWnd);
	void Draw(HDC hdc);
	void DrawTitle(HDC hdc);
	void DrawIssuesList(HDC hdc);
	void DrawSingleIssueCard(HDC hdc, const ISSUES_INFO& issue, RECT& cardRect, int index);
	void DrawProgressBar(HDC hdc, const ISSUES_INFO& issue, RECT& cardRect);

	COLORREF GetStatusColor(const std::string& status);
	COLORREF GetPriorityColor(const std::string& priority);
	std::wstring StringToWString(const std::string& str);

	void RequestIssues();

private:
	std::string ParsePyDictValueByKey(PyObject* dict, const char* key);

private:
	RECT m_TitleRect = { 0 };
	RECT m_IssuesRect = { 0 };
	LPCWSTR m_TitleText = L"Redmine Issues";



	std::vector<ISSUES_INFO> m_IssuesList;
};