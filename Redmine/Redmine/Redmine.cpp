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
	//if (m_Issues.empty()) return;

	// 设置issues区域的背景
	HBRUSH hBackgroundBrush = CreateSolidBrush(RGB(87, 192, 252));
	FillRect(hdc, &m_IssuesRect, hBackgroundBrush);
	DeleteObject(hBackgroundBrush);

	// 计算每个issue的显示区域
	int issueHeight = m_IssuesRect.bottom / 6; // 每个issue占1/6高度
	int padding = 10;

	//for (size_t i = 0; i < m_Issues.size() && i < 6; i++) {
	//	RECT issueRect = {
	//		m_IssuesRect.left + padding,
	//		m_IssuesRect.top + (int)i * issueHeight + padding,
	//		m_IssuesRect.right - padding,
	//		m_IssuesRect.top + (int)(i + 1) * issueHeight - padding
	//	};

	//	DrawSingleIssue(hdc, m_Issues[i], issueRect, (int)i + 1);
	//}
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
							// 提取字典中的各个字段
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

							// 构建issue显示字符串
							result += "\n" + std::to_string(i + 1) + ". 问题 #" + id + "\n";
							result += "   主题: " + subject + "\n";
							result += "   状态: " + status + "\n";
							result += "   优先级: " + priority + "\n";
							result += "   作者: " + author + "\n";
							result += "   分配给: " + (assigned_to.empty() ? "未分配" : assigned_to) + "\n";
							result += "   进度: " + done_ratio + "%\n";
							result += "   创建时间: " + created_on + "\n";
							result += "   更新时间: " + updated_on + "\n";

							if (!start_date.empty() && start_date != "None") {
								result += "   计划开始: " + start_date + "\n";
							}
							if (!due_date.empty() && due_date != "None") {
								result += "   计划完成: " + due_date + "\n";
							}

							result += "   项目: " + project + "\n";
							result += "   类型: " + tracker + "\n";

							if (!description.empty() && description != "None") {
								std::string desc_preview = description.length() > 150 ?
									description.substr(0, 150) + "..." : description;
								result += "   描述: " + desc_preview + "\n";
							}

							result += "--------------------------------------------------------------------------------\n";
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
