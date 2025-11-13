#pragma once

#include "resource.h"


class CManager {
public:
	CManager() {
		m_Redmine = new RedmineIssuesWidget();
		m_Logger = new Logger();
	}

	~CManager() {
		if (m_Redmine) delete m_Redmine;
		if (m_Logger) delete m_Logger;
	}

	RedmineIssuesWidget* GetRedmine() { return m_Redmine; }
	Logger* GetLogger() { return m_Logger; }

private:
	RedmineIssuesWidget* m_Redmine = nullptr;
	Logger* m_Logger = nullptr;
};