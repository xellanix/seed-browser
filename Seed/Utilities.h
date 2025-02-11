#pragma once
#ifndef UTILITIES_H
#define UTILITIES_H

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Shell32.lib")

#include <string>
#include <filesystem>
#include <Windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <sstream>
#include <fstream>
#include <ShlObj.h>
#include <array>
#include "..\..\..\Libraries\has_func.h"

namespace fs = std::filesystem;

namespace xellanix::utilities
{
	inline tm localtime_x(time_t _time)
	{
		struct tm _tm;
		localtime_s(&_tm, &_time);
		return _tm;
	}

	inline bool CheckExist(std::wstring const& path)
	{
		struct _stati64 buf;
		return _wstat64(path.c_str(), &buf) == 0;
	}
	inline bool CheckExist(fs::path const& path)
	{
		return CheckExist(path.wstring());
	}

	inline time_t FileDateCreated(std::wstring const& path)
	{
		struct _stati64 buf;
		if (_wstat64(path.c_str(), &buf) == 0)
		{
			return buf.st_ctime;
		}
		return 0;
	}
	inline time_t FileDateCreated(fs::path const& path)
	{
		return FileDateCreated(path.wstring());
	}

	inline time_t FileDateModified(std::wstring const& path)
	{
		struct _stati64 buf;
		if (_wstat64(path.c_str(), &buf) == 0)
		{
			return buf.st_mtime;
		}
		return 0;
	}
	inline time_t FileDateModified(fs::path const& path)
	{
		return FileDateModified(path.wstring());
	}

	inline std::wstring TimeString(struct tm _tm, std::wstring const& format)
	{
		std::wostringstream wss;
		wss << std::put_time(&_tm, format.c_str());
		return wss.str();
	}
	inline std::wstring TimeString(time_t toLocal, std::wstring const& format)
	{
		return TimeString(localtime_x(toLocal), format);
	}

	inline std::tuple<int, int, int> ParseDate(const std::wstring& date)
	{
		int day, month, year;
		std::wstringstream ss(date);
		ss >> day;
		ss.ignore(1); // Ignore the '-'
		ss >> month;
		ss.ignore(1); // Ignore the '-'
		ss >> year;

		return { day, month, year };
	}
	inline std::chrono::system_clock::time_point ToTimePoint(const std::wstring& date)
	{
		const auto&& [day, month, year] = ParseDate(date);

		// Construct a tm struct
		tm t = {};
		t.tm_mday = day;
		t.tm_mon = month - 1; // tm_mon is 0-based (January is 0)
		t.tm_year = year - 1900; // tm_year is years since 1900

		// Convert to system_clock::time_point
		auto tp = std::chrono::system_clock::from_time_t(mktime(&t));
		return tp;
	}
	inline int CountDays(const std::wstring& start, const std::wstring& end)
	{
		// Parse both dates into time_points
		std::chrono::system_clock::time_point tp1 = ToTimePoint(start);
		std::chrono::system_clock::time_point tp2 = ToTimePoint(end);

		// Calculate the difference in days
		auto duration = std::chrono::duration_cast<std::chrono::duration<int, std::ratio_multiply<std::ratio<24>, std::chrono::hours::period>>>(tp2 - tp1);
		return duration.count();
	}

	inline std::wstring GetAppDir()
	{
		DWORD path_buffer_size = MAX_PATH;
		std::unique_ptr<WCHAR[]> path_buf{ new WCHAR[path_buffer_size] };

		while (true)
		{
			const auto bytes_written = GetModuleFileName(NULL, path_buf.get(), path_buffer_size);
			const auto last_error = GetLastError();

			if (bytes_written == 0)
			{
				return std::wstring{};
			}

			if (last_error == ERROR_INSUFFICIENT_BUFFER)
			{
				path_buffer_size *= 2;
				path_buf.reset(new WCHAR[path_buffer_size]);
				continue;
			}
			
			auto path = std::wstring{ path_buf.get() };
			if (auto found = path.find_last_of(L"/\\"); found != std::wstring::npos)
			{
				path = path.substr(0, found);
			}

			return path;
		}
	}
	inline std::wstring AppDir = GetAppDir();

	inline fs::path GetLocalAppData(std::wstring const& appfolder = L"Xellanix")
	{
		fs::path path;
		PWSTR p_t;

		auto res = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &p_t);

		if (res != S_OK)
		{
			CoTaskMemFree(p_t);
			return path;
		}

		path = p_t;
		path /= appfolder;

		CoTaskMemFree(p_t);

		return path;
	}
	inline fs::path LocalAppData = GetLocalAppData(L"Xellanix\\Personal Space\\");
	inline fs::path LocalTemp = GetLocalAppData(L"Temp\\Xellanix\\Personal Space\\");

	inline bool IsProcessRunning(PROCESS_INFORMATION const& pi)
	{
		// Check if handle is closed
		if (pi.hProcess == NULL)
		{
			return false;
		}

		// If handle open, check if process is active
		DWORD lpExitCode = 0;
		if (GetExitCodeProcess(pi.hProcess, &lpExitCode) == 0)
		{
			return false;
		}
		else
		{
			return lpExitCode == STILL_ACTIVE;
		}
	}
	inline bool StopProcess(PROCESS_INFORMATION& pi)
	{
		// Check if handle is invalid or has allready been closed
		if (pi.hProcess == NULL)
		{
			return false;
		}

		// Terminate Process
		if (!TerminateProcess(pi.hProcess, 1))
		{
			return false;
		}

		// Close process and thread handles.
		if (!CloseHandle(pi.hProcess))
		{
			return false;
		}
		else
		{
			pi.hProcess = NULL;
		}

		if (!CloseHandle(pi.hThread))
		{
			return false;
		}
		else
		{
			pi.hProcess = NULL;
		}

		return true;
	}

	inline std::pair<double, std::wstring> NormalizeBytes(double const& bytes)
	{
		std::array<std::wstring, 9> scale{ L"B", L"KB", L"MB", L"GB", L"TB", L"PB", L"EB", L"ZB", L"YB" };

		unsigned short index = 0;
		double temp = bytes;

		while (temp >= 1024)
		{
			++index;
			temp /= 1024;

			if (index == 8)
				break;
		}

		return std::make_pair(temp, scale[index]);
	}
	inline std::pair<double, std::wstring> NormalizeBytes(unsigned long long const& bytes)
	{
		return NormalizeBytes(((double)bytes));
	}

	struct thousand_sep : std::numpunct<wchar_t>
	{
		char_type do_thousands_sep() const override { return L','; }
		std::string do_grouping() const override { return "\3"; }
	};
	inline std::wstring NumFormatter(double num)
	{
		std::wostringstream wss;
		auto thousands = std::make_unique<thousand_sep>();
		wss.imbue(std::locale(std::locale("C"), thousands.release()));
		wss << std::setprecision(2) << std::fixed << num;
		return wss.str();
	}

	inline std::wstring trim_spaces(const std::wstring& str, const std::wstring& whitespace = L" \t")
	{
		const auto strBegin = str.find_first_not_of(whitespace);
		if (strBegin == std::wstring::npos)
			return L""; // no content

		const auto strEnd = str.find_last_not_of(whitespace);
		const auto strRange = strEnd - strBegin + 1;

		return str.substr(strBegin, strRange);
	}

	inline std::wstring reduce_spaces(const std::wstring& str, const std::wstring& fill = L" ", const std::wstring& whitespace = L" \t")
	{
		// trim first
		auto result = trim_spaces(str, whitespace);

		// replace sub ranges
		auto beginSpace = result.find_first_of(whitespace);
		while (beginSpace != std::wstring::npos)
		{
			const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
			const auto range = endSpace - beginSpace;

			result.replace(beginSpace, range, fill);

			const auto newStart = beginSpace + fill.length();
			beginSpace = result.find_first_of(whitespace, newStart);
		}

		return result;
	}

	inline double round_up(double value, unsigned short decimal_places)
	{
		const double multiplier = std::pow(10.0, decimal_places);
		return std::round(value * multiplier) / multiplier;
	}

	template <typename T>
	inline std::optional<size_t> BinarySearch(std::vector<T> const& vec, T const& lookup)
	{
		size_t end = vec.size() - 1, start = 0;

		if (vec[start] == lookup) return start;
		else if (vec[end] == lookup) return end;

		while (start <= end)
		{
			auto mid = start + ((end - start) / 2);

			if (vec[mid] == lookup) return mid;
			else if (vec[mid] < lookup) start = mid + 1;
			else if (vec[mid] > lookup) end = mid - 1;
		}

		return std::nullopt;
	}
	template <typename T>
	inline std::optional<size_t> ReverseBinarySearch(std::vector<T> const& vec, T const& lookup)
	{
		size_t end = vec.size() - 1, start = 0;

		if (vec[start] == lookup) return start;
		else if (vec[end] == lookup) return end;

		while (start <= end)
		{
			auto mid = start + ((end - start) / 2);

			if (vec[mid] == lookup) return mid;
			else if (vec[mid] > lookup) start = mid + 1;
			else if (vec[mid] < lookup) end = mid - 1;
		}

		return std::nullopt;
	}
}

inline fs::path downloaded_update_path = L"";
inline unsigned long long downloaded_updated_size = 0;

namespace xellanix::desktop
{
	struct __declspec(uuid("45D64A29-A63E-4CB6-B498-5781D298CB4F")) __declspec(novtable)
		ICoreWindowInterop : public IUnknown
	{
	public:
		virtual HRESULT STDMETHODCALLTYPE get_WindowHandle(HWND* hwnd) = 0;
		virtual HRESULT STDMETHODCALLTYPE put_MessageHandled(unsigned char value) = 0;
	};

	struct __declspec(uuid("3E68D4BD-7135-4D10-8018-9FB6D9F33FA1")) __declspec(novtable)
		IInitializeWithWindow : public IUnknown
	{
	public:
		virtual HRESULT STDMETHODCALLTYPE Initialize(__RPC__in HWND hwnd) = 0;
	};

	inline HWND WindowHandle = nullptr;
}

inline bool get_ftime(std::wstring const& settingname, time_t& ctime, time_t& mtime, long long& size)
{
	struct _stati64 buf;
	if (_wstat64((xellanix::utilities::LocalAppData / settingname).wstring().c_str(), &buf) == 0)
	{
		ctime = buf.st_ctime;
		mtime = buf.st_mtime;
		size = buf.st_size;

		return true;
	}

	return false;
}

inline bool get_fattr(std::wstring const& path, time_t& mtime, long long& size)
{
	struct _stati64 buf;
	if (_wstat64(path.c_str(), &buf) == 0)
	{
		mtime = buf.st_mtime;
		size = buf.st_size;

		return true;
	}
	else
	{
		mtime = 0;
		size = 0;
	}

	return false;
}

template<typename... T>
inline std::array<typename std::common_type<T...>::type, sizeof...(T)> make_array(T&&...t)
{
	using first_t = typename std::common_type<T...>::type;
	return { std::forward<first_t>(t)... };
}

inline int natural_strcmp(const wchar_t* a, const wchar_t* b)
{
	int va = 0, vb = 0;

	while (true)
	{
		auto ca = a[va], cb = b[vb];

		while (iswspace(ca)) ca = a[++va];
		while (iswspace(cb)) cb = b[++vb];

		if (iswdigit(ca) && iswdigit(cb))
		{
			if (ca == L'0' || cb == L'0')
			{
				auto la = a + va;
				auto lb = b + vb;

				int result = 0;
				for (;; la++, lb++)
				{
					if (!iswdigit(*la) && !iswdigit(*lb))
					{
						result = 0;
						break;
					}
					if (!iswdigit(*la))
					{
						result = -1;
						break;
					}
					if (!iswdigit(*lb))
					{
						result = 1;
						break;
					}
					if (*la < *lb)
					{
						result = -1;
						break;
					}
					if (*la > *lb)
					{
						result = 1;
						break;
					}
				}

				if (result != 0) return result;
			}
			else
			{
				int result = [=]()
				{
					auto la = a + va;
					auto lb = b + vb;
					int bias = 0;

					for (;; la++, lb++)
					{
						if (!iswdigit(*la) && !iswdigit(*lb))
						{
							return bias;
						}
						if (!iswdigit(*la))
						{
							return -1;
						}
						if (!iswdigit(*lb))
						{
							return 1;
						}
						if (*la < *lb)
						{
							if (!bias) bias = -1;
						}
						else if (*la > *lb)
						{
							if (!bias) bias = 1;
						}
						else if (!*la && !*lb)
						{
							return bias;
						}
					}

					return 0;
				}();
				if (result != 0) return result;
			}
		}

		if (!ca && !cb) return 0;

		ca = towupper(ca);
		cb = towupper(cb);

		if (ca < cb) return -1;
		if (ca > cb) return 1;

		++va; ++vb;
	}
}

inline bool natural_less(const std::wstring& lhs, const std::wstring& rhs)
{
	return natural_strcmp(lhs.c_str(), rhs.c_str()) < 0;
}

template<typename C, typename T = std::decay_t<decltype(*begin(std::declval<C>()))>>
inline typename std::enable_if_t<std::is_same_v<T, std::wstring>> natural_sort(C& list)
{
	std::sort(list.begin(), list.end(), [](T const& lo, T const& ro)
	{
		return natural_less(lo, ro);
	});
}

template <typename T>
using has_swap = decltype(&T::swap);

template <typename T>
inline typename std::enable_if<is_detected_v<has_swap, T>>::type swapreset(T& obj)
{
	T{}.swap(obj);
}

#endif // !UTILITIES_H
