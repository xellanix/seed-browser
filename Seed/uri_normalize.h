#pragma once

#ifndef XELLANIX_URI_NORMALIZE_H
#define XELLANIX_URI_NORMALIZE_H

#include <cwctype>

#ifndef XELLANIX_STRING_UTILS_H
#include "string_utils.h"
#endif // !XELLANIX_STRING_UTILS_H

namespace xu = xellanix::utilities;

namespace xellanix::utilities
{
	inline std::wstring url_encode(std::wstring_view value)
	{
        // Convert the wide string to a UTF‑8 encoded narrow string.
        std::string utf8 = wide_to_utf8(value);

        // Pre-allocate a result string (worst-case: every byte becomes "%XX").
        std::string result;
        result.reserve(utf8.size() * 3);

        for (unsigned char c : utf8)
        {
            // RFC 3986 unreserved characters: A-Z, a-z, 0-9, hyphen, underscore, period, tilde.
            if ((c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') ||
                c == '-' || c == '_' || c == '.' || c == '~')
            {
                result.push_back(static_cast<char>(c));
            }
            else if (c == ' ')
            {
                // Replace space with plus sign.
                result.push_back('+');
            }
            else
            {
                char buf[4]; // Enough for "%XX" plus the null terminator.
                std::snprintf(buf, sizeof(buf), "%%%02X", c);
                result.append(buf, 3);
            }
        }
        // Convert the encoded narrow string back to a wide string.
        return utf8_to_wide(result);
	}

    // Using the default Google Search Engine url
    inline std::wstring normalize_uri(std::wstring_view input)
    {
        // --- Trim leading and trailing whitespace without allocation.
        size_t start = 0, end = input.size();
        while (start < end && std::iswspace(input[start]))
            ++start;
        while (end > start && std::iswspace(input[end - 1]))
            --end;
        std::wstring_view trimmed = input.substr(start, end - start);
        if (trimmed.empty())
            return L"";

        // --- Heuristic: determine if the input should be treated as a search query.
        bool containsSpace = false, containsDot = false;
        for (wchar_t ch : trimmed)
        {
            if (std::iswspace(ch))
                containsSpace = true;
            else if (ch == L'.')
                containsDot = true;
            if (containsSpace && containsDot)
                break;
        }
        if (containsSpace || !containsDot)
        {
            constexpr std::wstring_view defaultSearch = L"https://www.google.com/search?q=";
            return std::wstring(defaultSearch) + url_encode(trimmed);
        }

        // --- If the input already has a valid scheme, return it unchanged.
        if (starts_with(trimmed, L"http://") ||
            starts_with(trimmed, L"https://") ||
            starts_with(trimmed, L"ftp://"))
        {
            return std::wstring(trimmed);
        }

        // --- Otherwise, assume the URL is missing a scheme and prepend "http://".
        constexpr std::wstring_view httpPrefix = L"http://";
        return std::wstring(httpPrefix) + std::wstring(trimmed);
    }

    inline std::wstring normalize_uri_with_search_engine(std::wstring_view input, std::wstring_view searchEngineTemplate)
    {
        // --- Trim leading and trailing whitespace.
        size_t start = 0, end = input.size();
        while (start < end && std::iswspace(input[start]))
            ++start;
        while (end > start && std::iswspace(input[end - 1]))
            --end;
        std::wstring_view trimmed = input.substr(start, end - start);
        if (trimmed.empty())
            return L"";

        // --- Determine if the input should be treated as a search query.
        bool containsSpace = false, containsDot = false;
        for (wchar_t ch : trimmed)
        {
            if (std::iswspace(ch))
                containsSpace = true;
            else if (ch == L'.')
                containsDot = true;
            if (containsSpace && containsDot)
                break;
        }
        if (containsSpace || !containsDot)
        {
            // Treat as search query: URL‑encode the query.
            std::wstring encodedQuery = url_encode(trimmed);
            std::wstring result;
            // Process every occurrence of the placeholder "{}".
            size_t searchPos = 0;
            while (true)
            {
                size_t pos = searchEngineTemplate.find(L"{}", searchPos);
                if (pos == std::wstring_view::npos)
                {
                    // Append any remaining portion of the template.
                    result.append(searchEngineTemplate.substr(searchPos));
                    break;
                }
                // Append the segment before the placeholder.
                result.append(searchEngineTemplate.substr(searchPos, pos - searchPos));
                // Append the encoded query.
                result.append(encodedQuery);
                // Advance past the current placeholder.
                searchPos = pos + 2;
            }
            return result;
        }

        // --- Otherwise, treat the input as a URL.
        if (starts_with(trimmed, L"http://") ||
            starts_with(trimmed, L"https://") ||
            starts_with(trimmed, L"ftp://"))
        {
            return std::wstring(trimmed);
        }
        constexpr std::wstring_view httpPrefix = L"http://";
        return std::wstring(httpPrefix) + std::wstring(trimmed);
    }
}

#endif // !XELLANIX_URI_NORMALIZE_H