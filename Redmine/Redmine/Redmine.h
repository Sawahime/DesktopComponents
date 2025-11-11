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
	RedmineIssuesWidget() {
		FunctionEntryLog;
		InitializePython();
	}

	~RedmineIssuesWidget() {
		FunctionEntryLog;
		FinallizePython();
	}

	bool InitializePython();
	void FinallizePython();

	void InitWindowRectArea(HWND hWnd);
	void Draw(HDC hdc);
	void DrawTitle(HDC hdc);
	void DrawIssuesList(HDC hdc);
	void DrawSingleIssueCard(HDC hdc, const ISSUES_INFO& issue, RECT& cardRect);
	void DrawProgressBar(HDC hdc, const ISSUES_INFO& issue, RECT& cardRect);
	void HandleMouseWheel(int delta);

	void RequestIssues();

public:// Setter and Getter
	void SetWindowHandle(HWND hWnd) { m_hWnd = hWnd; }
	HWND GetWindowHandle() const { return m_hWnd; }

private:
	std::wstring StringToWString(const std::string& str);
	std::string WCharToString(const wchar_t* wstr);
	std::string ParsePyDictValueByKey(PyObject* dict, const char* key);
	void SortIssues();

private:
	HWND m_hWnd = nullptr;

	PyObject* m_Module = nullptr;
	PyObject* m_Func = nullptr;
	PyObject* m_ArgsTuple = nullptr;

	RECT m_TitleRect = { 0 };
	LPCWSTR m_TitleText = L"Redmine Issues";

	RECT m_IssuesRect = { 0 };
	int m_ContentStartYOffset = 0;
	int m_TotalContentHeight = 0;
	std::vector<ISSUES_INFO> m_IssuesList;
};