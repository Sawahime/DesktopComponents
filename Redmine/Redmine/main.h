#pragma once

std::wstring StringToWString(const std::string& str);
std::string WStringToString(const std::wstring& wstr);
std::string WCharToString(const wchar_t* wstr);