#pragma once

constexpr auto MAX_LOADSTRING = 100;

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
	RedmineIssuesWidget(HINSTANCE hInstance) :m_hInstance(hInstance) {
		FunctionEntryLog;

		m_Logger = new Logger();
		m_User = new RedmineUser();

		InitMessageFunctionTable();
		LoadStringW(hInstance, IDS_APP_TITLE, m_szTitle, MAX_LOADSTRING);
		LoadStringW(hInstance, IDC_REDMINE, m_szWindowClass, MAX_LOADSTRING);
		InitializePython();
	}

	~RedmineIssuesWidget() {
		FunctionEntryLog;

		if (m_User) delete m_User;
		if (m_Logger) delete m_Logger;

		FinallizePython();
	}

	void InitMessageFunctionTable();

	ATOM RegisterWindowClass() const;
	bool InitInstance(int nCmdShow);

	void InitWindowRectArea(HWND hWnd);
	void Draw(HDC hdc);
	void DrawTitle(HDC hdc);
	void DrawIssuesList(HDC hdc);
	void DrawSingleIssueCard(HDC hdc, const ISSUES_INFO& issue, RECT& cardRect);
	void DrawProgressBar(HDC hdc, const ISSUES_INFO& issue, RECT& cardRect);
	void NewDrawIssuesList(HDC);
	void NewDrawSingleIssueCard(HDC, const json&, RECT&);
	void NewDrawProgressBar(HDC, const json&, RECT&);

	bool InitializePython();
	void FinallizePython();
	void RequestIssues();

	json get_issues(int, int);
	json get_all_issues(int);
	json get_all_issues_by_assignee_name(std::string);
	void testrequest();

public:// Setter and Getter
	void SetWindowHandle(HWND hWnd) { m_hWnd = hWnd; }
	HWND GetWindowHandle() const { return m_hWnd; }

private:
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK MouseProc(int, WPARAM, LPARAM);

	std::unordered_map<UINT, std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>> m_MessageTable;
	LRESULT EvtCreateWindow(HWND, UINT, WPARAM, LPARAM);
	LRESULT EvtCommand(HWND, UINT, WPARAM, LPARAM);
	LRESULT EvtPaint(HWND, UINT, WPARAM, LPARAM);
	LRESULT EvtTimer(HWND, UINT, WPARAM, LPARAM);
	LRESULT EvtTrayNotify(HWND, UINT, WPARAM, LPARAM) const;
	LRESULT EvtDestroyWindow(HWND, UINT, WPARAM, LPARAM);

	void InitNotifyIconData(HWND hWnd);
	void DeInitNotifyIconData();
	void HandleMouseWheel(int delta);

	std::string ParsePyDictValueByKey(PyObject* dict, const char* key);
	void SortIssues();

private:
	std::string m_HostUrl = "192.168.3.202";
	int m_Port = 3000;

	HINSTANCE m_hInstance = nullptr;

	WCHAR m_szWindowClass[MAX_LOADSTRING] = { 0 };
	WCHAR m_szTitle[MAX_LOADSTRING] = { 0 };
	HWND m_hWnd = nullptr;

	NOTIFYICONDATA m_NotifyIconData = { 0 };
	HMENU m_hTrayMenu = nullptr;

	HHOOK m_hMouseHook;
	BYTE m_Opacity = 128;

	PyObject* m_Module = nullptr;
	PyObject* m_Func = nullptr;
	PyObject* m_ArgsTuple = nullptr;
	UINT_PTR m_TimerId = 1;
	UINT m_TimerIntervalMs = 30000;

#pragma region Layout
	RECT m_TitleRect = { 0 };
	LPCWSTR m_TitleText = L"Redmine Issues";

	RECT m_IssuesRect = { 0 };
	int m_ContentStartYOffset = 0;// for scroll
	int m_TotalContentHeight = 0;// for scroll
	std::vector<ISSUES_INFO> m_IssuesList;
	json m_JsonIssues;
#pragma endregion

	Logger* m_Logger = nullptr;
	RedmineUser* m_User = nullptr;
};