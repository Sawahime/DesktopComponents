#pragma once

constexpr auto MAX_LOADSTRING = 100;

class RedmineIssuesWidget {
public:
	RedmineIssuesWidget(HINSTANCE hInstance) :m_hInstance(hInstance) {
		FunctionEntryLog;

		m_Logger = new Logger();
		m_User = new RedmineUser();

		InitMessageFunctionTable();
		LoadStringW(hInstance, IDS_APP_TITLE, m_szTitle, MAX_LOADSTRING);
		LoadStringW(hInstance, IDC_REDMINE, m_szWindowClass, MAX_LOADSTRING);
	}

	~RedmineIssuesWidget() {
		FunctionEntryLog;

		if (m_User) delete m_User;
		if (m_Logger) delete m_Logger;
	}

	void InitMessageFunctionTable();

	ATOM RegisterWindowClass() const;
	bool InitInstance(int nCmdShow);

	void InitWindowRectArea(HWND hWnd);
	void Draw(HDC);
	void DrawTitle(HDC);
	void DrawIssuesList(HDC);
	void DrawSingleIssueCard(HDC, const json&, RECT&);
	void DrawProgressBar(HDC, const json&, RECT&);

	json HttpGetIssues(int, int);
	json HttpGetAllIssues(int);
	json HttpGetAllIssues(std::string);
	void RequestIssues();

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

	UINT_PTR m_RequestTimerId = 1;
	UINT m_RequestTimerIntervalMs = 3000;// [USER_TIMER_MINIMUM, USER_TIMER_MAXIMUM]

#pragma region Layout
	RECT m_TitleRect = { 0 };
	LPCWSTR m_TitleText = L"Redmine Issues";

	RECT m_IssuesRect = { 0 };
	int m_ContentStartYOffset = 0;// for scroll
	int m_TotalContentHeight = 0;// for scroll
	json m_Issues;
#pragma endregion

	Logger* m_Logger = nullptr;
	RedmineUser* m_User = nullptr;
};