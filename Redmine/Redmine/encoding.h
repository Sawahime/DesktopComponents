#pragma once

class EncodingConverter {
public:
	// UTF-8 → 本地编码
	static std::string utf8_to_local(const std::string& utf8_str) {
		if (utf8_str.empty()) return "";

		// UTF-8 → UTF-16 (Windows 原生 Unicode 格式)
		int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8_str.c_str(), (int)utf8_str.length(), nullptr, 0);
		if (wlen == 0) {
			throw std::runtime_error("UTF-8 to UTF-16 conversion failed");
		}
		std::wstring wstr(wlen, 0);
		wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8_str.c_str(), (int)utf8_str.length(), &wstr[0], wlen);
		if (wlen == 0) {
			throw std::runtime_error("UTF-8 to UTF-16 conversion failed");
		}

		// UTF-16 → 本地编码
		int len = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wstr.c_str(), (int)wstr.length(), nullptr, 0, nullptr, nullptr);
		if (len == 0) {
			throw std::runtime_error("UTF-16 to local encoding conversion failed");
		}
		std::string local_str(len, 0);
		len = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wstr.c_str(), (int)wstr.length(), &local_str[0], len, nullptr, nullptr);
		if (len == 0) {
			throw std::runtime_error("UTF-16 to local encoding conversion failed");
		}

		return local_str;
	}

	// 本地编码 → UTF-8
	static std::string local_to_utf8(const std::string& local_str) {
		if (local_str.empty()) return "";

		// 本地编码 → UTF-16
		int wlen = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, local_str.c_str(), (int)local_str.length(), nullptr, 0);
		if (wlen == 0) {
			throw std::runtime_error("Local encoding to UTF-16 conversion failed");
		}
		std::wstring wstr(wlen, 0);
		wlen = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, local_str.c_str(), (int)local_str.length(), &wstr[0], wlen);
		if (wlen == 0) {
			throw std::runtime_error("Local encoding to UTF-16 conversion failed");
		}

		// UTF-16 → UTF-8
		int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), nullptr, 0, nullptr, nullptr);
		if (len == 0) {
			throw std::runtime_error("UTF-16 to UTF-8 conversion failed");
		}
		std::string utf8_str(len, 0);
		len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), &utf8_str[0], len, nullptr, nullptr);
		if (len == 0) {
			throw std::runtime_error("UTF-16 to UTF-8 conversion failed");
		}

		return utf8_str;
	}
};