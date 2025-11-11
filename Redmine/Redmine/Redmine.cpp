#include "framework.h"
#include "redmine.h"
#include <iostream>
#include <unordered_map>
#include <algorithm>


bool RedmineIssuesWidget::InitializePython() {
	Py_Initialize();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('.')");

	m_Module = PyImport_ImportModule("pyif");// python file name
	if (m_Module == nullptr) {
		DebugPrint(L"错误：无法导入模块 pyif" << std::endl);
		return false;
	}

	m_Func = PyObject_GetAttrString(m_Module, "get_issues_by_assignee_name_cpp_intf");
	if (m_Func == nullptr || PyCallable_Check(m_Func) == false) {
		DebugPrint(L"错误：找不到函数或函数不可调用" << std::endl);
		return false;
	}

	// Prepare the input param
	m_ArgsTuple = PyTuple_New(1);// new an empty tuple
	if (m_ArgsTuple == nullptr) {
		DebugPrint(L"错误：PyTuple_New failed" << std::endl);
		return false;
	}
	PyObject* arg1 = PyUnicode_FromWideChar(L"毅 陆", -1);
	if (arg1 == nullptr) {
		DebugPrint(L"错误：PyUnicode_FromWideChar failed" << std::endl);
		return false;
	}
	// Inser pName to the tuple. Tuple steals reference, so do not Py_DECREF(Decrease Reference)
	PyTuple_SetItem(m_ArgsTuple, 0, arg1);

	return true;
}


void RedmineIssuesWidget::FinallizePython() {
	if (m_ArgsTuple) {
		Py_DECREF(m_ArgsTuple);
		m_ArgsTuple = nullptr;
	}
	if (m_Func) {
		Py_DECREF(m_Func);
		m_Func = nullptr;
	}
	if (m_Module) {
		Py_DECREF(m_Module);
		m_Module = nullptr;
	}
	Py_Finalize();
}


void RedmineIssuesWidget::InitWindowRectArea(HWND hWnd) {
	GetClientRect(hWnd, &m_TitleRect);
	m_TitleRect.bottom /= 6;

	GetClientRect(hWnd, &m_IssuesRect);
	m_IssuesRect.top = m_TitleRect.bottom;
}


void RedmineIssuesWidget::Draw(HDC hdc) {
	DrawTitle(hdc);
	DrawIssuesList(hdc);
}


void RedmineIssuesWidget::DrawTitle(HDC hdc) {
	// Title
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(0, 0, 0));
	HFONT hFont = CreateFontW(
		36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI"
	);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
	DrawTextW(hdc, m_TitleText, -1, &m_TitleRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);
}


void RedmineIssuesWidget::DrawIssuesList(HDC hdc) {
	// Set the background of the issues area
	HBRUSH hBackgroundBrush = CreateSolidBrush(RGB(87, 192, 252));
	FillRect(hdc, &m_IssuesRect, hBackgroundBrush);
	DeleteObject(hBackgroundBrush);

	if (m_IssuesList.empty()) return;

	// Calculate the size and spacing of the cards
	int margins = 10; // Left and right margins (in px).
	int cardWidth = m_IssuesRect.right - m_IssuesRect.left - margins * 2;
	int cardHeight = 72;
	int cardSpacing = 10; // The spacing between the cards
	int startY = m_IssuesRect.top + 10 + m_ContentStartYOffset;

	m_TotalContentHeight = (int)m_IssuesList.size() * (cardHeight + cardSpacing);

	for (size_t i = 0; i < m_IssuesList.size(); i++) {
		// Calculate the start y-coordinate of each card
		int cardY = startY + (int)i * (cardHeight + cardSpacing);

		// Check if the card is within the visible area
		if (cardY + cardHeight < m_IssuesRect.top || cardY > m_IssuesRect.bottom) {
			continue; // Not within the visible area, skip the drawing.
		}

		// Define the rectangular area of the card
		RECT cardRect = {
			m_IssuesRect.left + margins,// left
			cardY,// top
			m_IssuesRect.left + margins + cardWidth,// right
			cardY + cardHeight// bottom
		};

		DrawSingleIssueCard(hdc, m_IssuesList[i], cardRect);
	}
}


void RedmineIssuesWidget::DrawSingleIssueCard(HDC hdc, const ISSUES_INFO& issue, RECT& cardRect) {
	// Draw the background of the card
	HBRUSH hCardBrush = CreateSolidBrush(RGB(255, 255, 255));
	HPEN hBorderPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hCardBrush);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hBorderPen);
	RoundRect(hdc, cardRect.left, cardRect.top, cardRect.right, cardRect.bottom, 12, 12);
	SelectObject(hdc, hOldBrush);
	SelectObject(hdc, hOldPen);
	DeleteObject(hCardBrush);
	DeleteObject(hBorderPen);

	// Set text properties
	SetBkMode(hdc, TRANSPARENT);
	// Create font
	HFONT hBoldFont = CreateFontW(
		20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei UI"
	);
	HFONT hNormalFont = CreateFontW(
		14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei UI"
	);
	HFONT hSmallFont = CreateFontW(
		12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei UI"
	);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hBoldFont);

	// Draw the ID and subject
	SetTextColor(hdc, RGB(70, 130, 180));
	std::wstring subjectText = L"#" + StringToWString(issue.id) + L" " + StringToWString(issue.subject);
	RECT subjectRect = { cardRect.left + 15, cardRect.top + 12, cardRect.right - 15, cardRect.top + 35 };
	DrawTextW(hdc, subjectText.c_str(), -1, &subjectRect, DT_LEFT | DT_SINGLELINE);

	// 绘制进度条
	DrawProgressBar(hdc, issue, cardRect);

	SelectObject(hdc, hOldFont);
	DeleteObject(hBoldFont);
	DeleteObject(hNormalFont);
	DeleteObject(hSmallFont);
}


void RedmineIssuesWidget::DrawProgressBar(HDC hdc, const ISSUES_INFO& issue, RECT& cardRect) {
	int actualProgress = 0;
	if (!issue.done_ratio.empty() && issue.done_ratio != "None") {
		actualProgress = std::stoi(issue.done_ratio);
	}

	// 进度条位置和尺寸
	int margins = 15;
	int barWidth = cardRect.right - cardRect.left - margins * 2 - 48;;
	int barHeight = 16;
	int barX = cardRect.left + margins;
	int barY = cardRect.bottom - barHeight - 12;

	// 绘制进度条背景
	HBRUSH hBgBrush = CreateSolidBrush(RGB(240, 240, 240));
	HPEN hBorderPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBgBrush);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hBorderPen);
	Rectangle(hdc, barX, barY, barX + barWidth, barY + barHeight);

	if (!issue.start_date.empty() && issue.start_date != "None" &&
		!issue.due_date.empty() && issue.due_date != "None")
	{
		SYSTEMTIME startDate = { 0 };
		sscanf_s(issue.start_date.c_str(), "%hu-%hu-%hu", &startDate.wYear, &startDate.wMonth, &startDate.wDay);

		SYSTEMTIME dueDate = { 0 };
		sscanf_s(issue.due_date.c_str(), "%hu-%hu-%hu", &dueDate.wYear, &dueDate.wMonth, &dueDate.wDay);

		SYSTEMTIME currentDate;
		GetLocalTime(&currentDate);

		// 将 SYSTEMTIME 转换为 FILETIME 以便计算
		FILETIME ftStart, ftDue, ftCurrent;
		SystemTimeToFileTime(&startDate, &ftStart);
		SystemTimeToFileTime(&dueDate, &ftDue);
		SystemTimeToFileTime(&currentDate, &ftCurrent);

		// 将 FILETIME 转换为 ULARGE_INTEGER 进行数值计算
		ULARGE_INTEGER ullStart = { 0 };
		ullStart.LowPart = ftStart.dwLowDateTime;
		ullStart.HighPart = ftStart.dwHighDateTime;

		ULARGE_INTEGER ullDue = { 0 };
		ullDue.LowPart = ftDue.dwLowDateTime;
		ullDue.HighPart = ftDue.dwHighDateTime;

		ULARGE_INTEGER ullCurrent = { 0 };
		ullCurrent.LowPart = ftCurrent.dwLowDateTime;
		ullCurrent.HighPart = ftCurrent.dwHighDateTime;

		int theoreticalProgress;

		if (ullCurrent.QuadPart <= ullStart.QuadPart || ullDue.QuadPart <= ullStart.QuadPart) {
			theoreticalProgress = 0;
		}
		else {
			theoreticalProgress = (ullCurrent.QuadPart - ullStart.QuadPart) * 100 / (ullDue.QuadPart - ullStart.QuadPart);
			theoreticalProgress = min(theoreticalProgress, 100);
		}

		// 绘制理论进度（红色）
		HBRUSH hTheoreticalBrush = CreateSolidBrush(RGB(255, 100, 100));
		SelectObject(hdc, hTheoreticalBrush);
		Rectangle(hdc, barX, barY, barX + barWidth * theoreticalProgress / 100, barY + barHeight);
		DeleteObject(hTheoreticalBrush);
	}

	// 绘制实际进度（绿色）
	int actualWidth = (barWidth * actualProgress) / 100;
	HBRUSH hActualBrush = CreateSolidBrush(RGB(50, 205, 50));
	SelectObject(hdc, hActualBrush);
	Rectangle(hdc, barX, barY, barX + actualWidth, barY + barHeight);

	// 绘制进度文本
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(100, 100, 100));
	HFONT hSmallFont = CreateFontW(
		barHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei UI"
	);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hSmallFont);
	std::wstring progressText = std::to_wstring(actualProgress) + L"% 完成";
	RECT textRect = { barX + barWidth + 10, barY - 2, barX + barWidth + 150, barY + barHeight + 2 };
	DrawTextW(hdc, progressText.c_str(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	SelectObject(hdc, hOldBrush);
	SelectObject(hdc, hOldPen);
	SelectObject(hdc, hOldFont);
	DeleteObject(hActualBrush);
	DeleteObject(hBgBrush);
	DeleteObject(hBorderPen);
	DeleteObject(hSmallFont);
}


std::wstring RedmineIssuesWidget::StringToWString(const std::string& str) {
	if (str.empty()) return L"";

	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
	if (len == 0) return L"";

	std::wstring wstr(len - 1, 0);
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &wstr[0], len);
	return wstr;
}

std::string RedmineIssuesWidget::WCharToString(const wchar_t* wstr) {
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


void RedmineIssuesWidget::HandleMouseWheel(int delta)
{
	int scrollAmount = -delta / WHEEL_DELTA; // Change to scroll rows
	int offset = m_ContentStartYOffset - scrollAmount * 30; // Scroll 30 pixels each time

	// upperLimit <= start_y_offset <= lowerLimit
	int upperLimit = (m_IssuesRect.bottom - m_IssuesRect.top) - m_TotalContentHeight;
	int lowerLimit = 0;
	offset = max(upperLimit, offset);
	offset = min(lowerLimit, offset);

	if (offset != m_ContentStartYOffset) {
		m_ContentStartYOffset = offset;

		// Redraw
		if (m_hWnd) {
			InvalidateRect(m_hWnd, &m_IssuesRect, TRUE);// it will trigger case WM_PAINT to redraw
		}
	}
}


void RedmineIssuesWidget::RequestIssues() {
	if (m_Func == nullptr) {
		DebugPrint(L"错误：m_Func为空" << std::endl);
		return;
	}
	if (m_ArgsTuple == nullptr) {
		DebugPrint(L"错误：m_ArgsTuple为空" << std::endl);
		return;
	}

	m_IssuesList.clear();

	PyObject* pResult = PyObject_CallObject(m_Func, m_ArgsTuple);
	if (pResult == nullptr || PyList_Check(pResult) == false) {
		DebugPrint(L"错误：PyObject_CallObject failed or result is not a list" << std::endl);
		return;
	}

	Py_ssize_t count = PyList_Size(pResult);
	std::string result = "Totally " + std::to_string(count) + " issues for the assignor\n";
	result += "================================================================================\n";

	for (Py_ssize_t i = 0; i < count; i++) {
		PyObject* pIssue = PyList_GetItem(pResult, i);
		if (PyDict_Check(pIssue) == false) {
			continue;
		}

		// Extract each field from the dictionary
		std::string id = ParsePyDictValueByKey(pIssue, "id");
		std::string subject = ParsePyDictValueByKey(pIssue, "subject");
		std::string status = ParsePyDictValueByKey(pIssue, "status");
		std::string priority = ParsePyDictValueByKey(pIssue, "priority");
		std::string author = ParsePyDictValueByKey(pIssue, "author");
		std::string assigned_to = ParsePyDictValueByKey(pIssue, "assigned_to");
		std::string done_ratio = ParsePyDictValueByKey(pIssue, "done_ratio");
		std::string created_on = ParsePyDictValueByKey(pIssue, "created_on");
		std::string updated_on = ParsePyDictValueByKey(pIssue, "updated_on");
		std::string start_date = ParsePyDictValueByKey(pIssue, "start_date");
		std::string due_date = ParsePyDictValueByKey(pIssue, "due_date");
		std::string project = ParsePyDictValueByKey(pIssue, "project");
		std::string tracker = ParsePyDictValueByKey(pIssue, "tracker");
		std::string description = ParsePyDictValueByKey(pIssue, "description");

		// Debug Info
		result += "\n" + std::to_string(i + 1) + ". 问题 #" + id + "\n";
		result += "   主题: " + subject + "\n";
		result += "   状态: " + status + "\n";
		result += "   优先级: " + priority + "\n";
		result += "   作者: " + author + "\n";
		result += "   分配给: " + assigned_to + "\n";
		result += "   进度: " + done_ratio + "%\n";
		result += "   创建时间: " + created_on + "\n";
		result += "   更新时间: " + updated_on + "\n";
		result += "   计划开始: " + start_date + "\n";
		result += "   计划完成: " + due_date + "\n";
		result += "   项目: " + project + "\n";
		result += "   类型: " + tracker + "\n";
		std::string desc_preview = description.length() > 150 ? description.substr(0, 150) + "..." : description;
		result += "   描述: " + desc_preview + "\n";
		result += "--------------------------------------------------------------------------------\n";
		std::cout << result << std::endl;
		result.clear();

		ISSUES_INFO issue;
		issue.id = id;
		issue.subject = subject;
		issue.status = status;
		issue.priority = priority;
		issue.done_ratio = done_ratio;
		issue.start_date = start_date;
		issue.due_date = due_date;
		m_IssuesList.push_back(issue);
	}

	SortIssues();

	if (pResult) Py_DECREF(pResult);
}


std::string RedmineIssuesWidget::ParsePyDictValueByKey(PyObject* dict, const char* key) {
	PyObject* pValue = PyDict_GetItemString(dict, key);
	if (pValue) {
		if (PyUnicode_Check(pValue)) {
			PyObject* temp_bytes = PyUnicode_AsEncodedString(pValue, "ANSI", "strict");
			if (temp_bytes != NULL) {
				std::string result = PyBytes_AS_STRING(temp_bytes);
				Py_DECREF(temp_bytes);
				return result;
			}
		}
		else if (PyLong_Check(pValue)) {
			long value = PyLong_AsLong(pValue);
			return std::to_string(value);
		}
		else if (PyFloat_Check(pValue)) {
			double value = PyFloat_AsDouble(pValue);
			return std::to_string(static_cast<int>(value));
		}
	}
	return "";
}


void RedmineIssuesWidget::SortIssues() {
	std::unordered_map<std::string, int> priorityMap = {
		{"Immediate", 0},
		{"Urgent", 1},
		{"High", 2},
		{"Normal", 3},
		{"Low", 4}
	};

	std::stable_sort(
		m_IssuesList.begin(), m_IssuesList.end(),
		[](const ISSUES_INFO& a, const ISSUES_INFO& b) {
			if (a.due_date.empty() && b.due_date.empty()) return false;
			if (a.due_date.empty()) return false;  // a空，b不空，a排后面
			if (b.due_date.empty()) return true;   // a不空，b空，a排前面
			return a.due_date < b.due_date;
		}
	);

	std::stable_sort(
		m_IssuesList.begin(), m_IssuesList.end(),
		[&priorityMap](const ISSUES_INFO& a, const ISSUES_INFO& b) {
			int priorityA = priorityMap.count(a.priority) ? priorityMap[a.priority] : 4;
			int priorityB = priorityMap.count(b.priority) ? priorityMap[b.priority] : 4;
			return priorityA < priorityB;
		}
	);
}