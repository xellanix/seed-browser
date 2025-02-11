#pragma once

#ifndef XELLANIX_STRING_UTILS_H
#define XELLANIX_STRING_UTILS_H

#include <string>
#include <string_view>
#include <Windows.h>

namespace xu = xellanix::utilities;

namespace xellanix::utilities
{
	inline constexpr bool starts_with(std::wstring_view str, std::wstring_view prefix) noexcept
	{
		return str.size() >= prefix.size() && str.substr(0, prefix.size()) == prefix;
	}

	inline std::string wide_to_utf8(std::wstring_view wstr)
    {
        if (wstr.empty())
            return std::string();
        // Compute the required buffer size in bytes.
        int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
        if (sizeNeeded <= 0)
            return std::string();
        std::string utf8(sizeNeeded, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), &utf8[0], sizeNeeded, nullptr, nullptr);
        return utf8;
    }

    inline std::wstring utf8_to_wide(std::string_view str)
    {
        if (str.empty())
            return std::wstring();
        // Compute the required buffer size in wide characters.
        int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
        if (sizeNeeded <= 0)
            return std::wstring();
        std::wstring wide(sizeNeeded, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), &wide[0], sizeNeeded);
        return wide;
    }
}

#endif // !XELLANIX_STRING_UTILS_H