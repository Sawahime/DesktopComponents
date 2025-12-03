#pragma once


#define WM_USER_INFO_UPDATE (WM_USER + 2)


std::wstring StringToWString(const std::string& str);
std::string WStringToString(const std::wstring& wstr);
std::string WCharToString(const wchar_t* wstr);
std::string JsonToString(const json& j);