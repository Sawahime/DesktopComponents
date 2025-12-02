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
		InitMessageFunctionTable();
		LoadStringW(hInstance, IDS_APP_TITLE, m_szTitle, MAX_LOADSTRING);
		LoadStringW(hInstance, IDC_REDMINE, m_szWindowClass, MAX_LOADSTRING);
		InitializePython();

		m_Logger = new Logger();
		m_User = new RedmineUser();
	}

	~RedmineIssuesWidget() {
		FunctionEntryLog;
		FinallizePython();

		if (m_User) delete m_User;
		if (m_Logger) delete m_Logger;
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

	bool InitializePython();
	void FinallizePython();
	void RequestIssues();

	json get_issues(int, int);
	json get_all_issues(int);
	json get_all_issues_by_assignee_name(std::string);
	void testrequest();

	bool SaveUserInfo(const std::wstring&, const std::string&, const std::string&);
	bool LoadUserInfo(const std::wstring&, std::string&, std::string&);
	bool HasSavedUserInfo(const std::wstring&);
	bool DeleteUserInfo(const std::wstring&);

public:// Setter and Getter
	void SetUserId(const int& id) { m_UserId = id; }
	void SetUserName(const std::string& name) { m_UserName = name; }
	void SetApiKey(const std::string& api_key) { m_ApiKey = api_key; }
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
	int m_UserId;
	std::string m_UserName;
	std::string m_ApiKey;

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
#pragma endregion

	Logger* m_Logger = nullptr;
	RedmineUser* m_User = nullptr;
};