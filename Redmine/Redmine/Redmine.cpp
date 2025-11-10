#include "framework.h"
#include "redmine.h"
#include <iostream>

using namespace std;


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

	// Boundary
	HPEN hPen = CreatePen(PS_SOLID, 2, RGB(200, 200, 200));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	MoveToEx(hdc, m_TitleRect.left + 10, m_TitleRect.bottom, nullptr);// move pan to (x,y)
	LineTo(hdc, m_TitleRect.right - 10, m_TitleRect.bottom);// draw line from current position to target position
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
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
	int cardHeight = 120;
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

		DrawSingleIssueCard(hdc, m_IssuesList[i], cardRect, (int)i + 1);
	}
}


void RedmineIssuesWidget::DrawSingleIssueCard(HDC hdc, const ISSUES_INFO& issue, RECT& cardRect, int index) {
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
		16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
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

	// 绘制状态标签（右上角）
	SelectObject(hdc, hSmallFont);
	COLORREF statusColor = GetStatusColor(issue.status);
	SetTextColor(hdc, statusColor);
	RECT statusRect = { cardRect.right - 100, cardRect.top + 15, cardRect.right - 15, cardRect.top + 35 };
	std::wstring statusText = L"状态: " + StringToWString(issue.status);
	DrawTextW(hdc, statusText.c_str(), -1, &statusRect, DT_RIGHT | DT_SINGLELINE);

	// 绘制优先级标签
	COLORREF priorityColor = GetPriorityColor(issue.priority);
	SetTextColor(hdc, priorityColor);
	RECT priorityRect = { cardRect.right - 100, cardRect.top + 35, cardRect.right - 15, cardRect.top + 55 };
	std::wstring priorityText = L"优先级: " + StringToWString(issue.priority);
	DrawTextW(hdc, priorityText.c_str(), -1, &priorityRect, DT_RIGHT | DT_SINGLELINE);

	// 绘制进度条
	DrawProgressBar(hdc, issue, cardRect);

	// 绘制日期信息（底部）
	SetTextColor(hdc, RGB(100, 100, 100));
	if (!issue.start_date.empty() && issue.start_date != "None") {
		RECT dateRect = { cardRect.left + 15, cardRect.bottom - 25, cardRect.right - 15, cardRect.bottom - 5 };
		std::wstring dateText = L"开始: " + StringToWString(issue.start_date);
		if (!issue.due_date.empty() && issue.due_date != "None") {
			dateText += L" | 截止: " + StringToWString(issue.due_date);
		}
		DrawTextW(hdc, dateText.c_str(), -1, &dateRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
	}

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

	int theoreticalProgress = 0;

	// 进度条位置和尺寸
	int barWidth = 200;
	int barHeight = 12;
	int barX = cardRect.left + 15;
	int barY = cardRect.bottom - 40;

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
	DeleteObject(hActualBrush);

	// 恢复原来的画笔和画刷
	SelectObject(hdc, hOldBrush);
	SelectObject(hdc, hOldPen);
	DeleteObject(hBgBrush);
	DeleteObject(hBorderPen);

	// 绘制进度文本
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(100, 100, 100));
	HFONT hSmallFont = CreateFontW(
		10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei UI"
	);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hSmallFont);
	std::wstring progressText = std::to_wstring(actualProgress) + L"% 完成";
	RECT textRect = { barX + barWidth + 10, barY - 2, barX + barWidth + 150, barY + barHeight + 2 };
	DrawTextW(hdc, progressText.c_str(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

Clearup:
	SelectObject(hdc, hOldFont);
	DeleteObject(hSmallFont);
}


COLORREF RedmineIssuesWidget::GetStatusColor(const std::string& status) {
	if (status.find("新") != std::string::npos || status.find("New") != std::string::npos) {
		return RGB(70, 130, 180); // 蓝色
	}
	else if (status.find("进行") != std::string::npos || status.find("Progress") != std::string::npos) {
		return RGB(255, 140, 0); // 橙色
	}
	else if (status.find("完成") != std::string::npos || status.find("Resolved") != std::string::npos) {
		return RGB(50, 205, 50); // 绿色
	}
	else {
		return RGB(100, 100, 100); // 灰色
	}
}


COLORREF RedmineIssuesWidget::GetPriorityColor(const std::string& priority) {
	if (priority.find("高") != std::string::npos || priority.find("High") != std::string::npos) {
		return RGB(220, 80, 60); // 红色
	}
	else if (priority.find("中") != std::string::npos || priority.find("Normal") != std::string::npos) {
		return RGB(255, 165, 0); // 橙色
	}
	else {
		return RGB(50, 205, 50); // 绿色
	}
}


std::wstring RedmineIssuesWidget::StringToWString(const std::string& str) {
	if (str.empty()) return L"";

	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
	if (len == 0) return L"";

	std::wstring wstr(len - 1, 0);
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &wstr[0], len);
	return wstr;
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
	Py_Initialize();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('.')");

	// 导入模块
	PyObject* pModule = PyImport_ImportModule("pyif");
	if (pModule != NULL) {
		// 获取函数
		PyObject* pFunc = PyObject_GetAttrString(pModule, "get_issues_cpp_intf");
		if (pFunc && PyCallable_Check(pFunc)) {
			// 调用函数
			PyObject* pResult = PyObject_CallObject(pFunc, NULL);
			if (pResult != NULL) {
				// 检查返回的是否是列表
				if (PyList_Check(pResult)) {
					Py_ssize_t count = PyList_Size(pResult);

					// 准备结果字符串
					std::string result = "分配给【毅 陆】的issues (总计: " + std::to_string(count) + " 个)\n";
					result += "====================================================================================================\n";

					for (Py_ssize_t i = 0; i < count; i++) {
						PyObject* pIssue = PyList_GetItem(pResult, i);
						if (PyDict_Check(pIssue)) {
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

							// 添加到 m_IssuesList
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
					}
					std::cout << result << std::endl;

				}
				else {
					// 处理错误情况
					DebugPrint(L"错误：函数返回的不是列表" << std::endl);
				}

				Py_DECREF(pResult);
			}
			Py_DECREF(pFunc);
		}
		else {
			DebugPrint(L"错误：找不到函数 get_issues_cpp_intf" << std::endl);
		}
		Py_DECREF(pModule);
	}
	else {
		DebugPrint(L"错误：无法导入模块 pyif" << std::endl);
	}

	Py_Finalize();
}


string RedmineIssuesWidget::ParsePyDictValueByKey(PyObject* dict, const char* key) {
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
