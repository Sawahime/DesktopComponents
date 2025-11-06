#include "framework.h"
#include "redmine.h"

void RedmineIssuesWidget::InitWindowRectArea(HWND hWnd) {
	GetClientRect(hWnd, &m_TitleRect);
	m_TitleRect.bottom /= 6;

	GetClientRect(hWnd, &m_IssuesRect);
	m_IssuesRect.top = m_TitleRect.bottom;
}

void RedmineIssuesWidget::Draw(HDC hdc) {
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