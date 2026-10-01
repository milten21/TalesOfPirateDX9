#pragma once

#ifdef _WIN32

#include <windows.h>

#else

#include <cctype>
#include <cerrno>
#include <chrono>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <thread>
#include <type_traits>

#include <strings.h>
#include <sys/time.h>
#include <unistd.h>

using BYTE = std::uint8_t;
using WORD = std::uint16_t;
using DWORD = std::uint32_t;
using BOOL = int;
using CHAR = char;
using UCHAR = unsigned char;
using SHORT = short;
using USHORT = unsigned short;
using UINT = unsigned int;
using LONG = std::int32_t;
using LONG32 = std::int32_t;
using ULONG = std::uint32_t;
using LONG64 = std::int64_t;
using ULONGLONG = std::uint64_t;
using DWORD64 = std::uint64_t;
using ULONG_PTR = std::uintptr_t;
using SIZE_T = std::size_t;
using WPARAM = std::uintptr_t;
using LPARAM = std::intptr_t;
using HANDLE = void*;
using HWND = void*;
using HMODULE = void*;
using HINSTANCE = void*;
using PVOID = void*;
using LPVOID = void*;
using LPCVOID = const void*;
using PCHAR = char*;
using LPSTR = char*;
using LPCSTR = const char*;
using LPBYTE = BYTE*;
using LPDWORD = DWORD*;
using TCHAR = char;
using _TCHAR = char;
using errno_t = int;

#define __int64 long long

#define VOID void
#define TRUE 1
#define FALSE 0
#define CALLBACK
#define WINAPI
#define APIENTRY

#define MAX_PATH 260
#define _MAX_PATH 260
#define INFINITE 0xFFFFFFFF
#define WAIT_OBJECT_0 0x00000000L
#define WAIT_TIMEOUT 0x00000102L
#define WAIT_FAILED 0xFFFFFFFF

#define THREAD_PRIORITY_IDLE (-15)
#define THREAD_PRIORITY_LOWEST (-2)
#define THREAD_PRIORITY_BELOW_NORMAL (-1)
#define THREAD_PRIORITY_NORMAL 0
#define THREAD_PRIORITY_ABOVE_NORMAL 1
#define THREAD_PRIORITY_HIGHEST 2
#define THREAD_PRIORITY_TIME_CRITICAL 15

#define LOBYTE(w) (static_cast<BYTE>(static_cast<std::uintptr_t>(w) & 0xff))
#define HIBYTE(w) (static_cast<BYTE>((static_cast<std::uintptr_t>(w) >> 8) & 0xff))
#define LOWORD(l) (static_cast<WORD>(static_cast<std::uintptr_t>(l) & 0xffff))
#define HIWORD(l) (static_cast<WORD>((static_cast<std::uintptr_t>(l) >> 16) & 0xffff))
#define MAKEWORD(a, b) (static_cast<WORD>(LOBYTE(a) | (static_cast<WORD>(LOBYTE(b)) << 8)))

#define ZeroMemory(destination, length) std::memset((destination), 0, (length))
#define CONTAINING_RECORD(address, type, field) \
	(reinterpret_cast<type*>(reinterpret_cast<char*>(address) - reinterpret_cast<ULONG_PTR>(&reinterpret_cast<type*>(0)->field)))

#define _TRUNCATE (static_cast<std::size_t>(-1))
#define STRUNCATE 80

struct RECT
{
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
};

struct SYSTEMTIME
{
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
};

namespace portability
{
inline void CurrentTime(SYSTEMTIME* time, std::tm* (*toCalendar)(const std::time_t*, std::tm*))
{
	timeval now{};
	gettimeofday(&now, nullptr);
	const std::time_t seconds = now.tv_sec;
	std::tm calendar{};
	toCalendar(&seconds, &calendar);
	time->wYear = static_cast<WORD>(calendar.tm_year + 1900);
	time->wMonth = static_cast<WORD>(calendar.tm_mon + 1);
	time->wDayOfWeek = static_cast<WORD>(calendar.tm_wday);
	time->wDay = static_cast<WORD>(calendar.tm_mday);
	time->wHour = static_cast<WORD>(calendar.tm_hour);
	time->wMinute = static_cast<WORD>(calendar.tm_min);
	time->wSecond = static_cast<WORD>(calendar.tm_sec);
	time->wMilliseconds = static_cast<WORD>(now.tv_usec / 1000);
}

inline int FormatTruncated(char* buffer, std::size_t size, const char* format, va_list args)
{
	if (!buffer || size == 0)
	{
		return -1;
	}
	const int length = std::vsnprintf(buffer, size, format, args);
	return (length < 0 || static_cast<std::size_t>(length) >= size) ? -1 : length;
}

template <typename Integer>
char* ToString(Integer value, char* buffer, int radix)
{
	constexpr char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	if (radix < 2 || radix > 36)
	{
		buffer[0] = '\0';
		return buffer;
	}
	using Unsigned = std::make_unsigned_t<Integer>;
	const bool negative = radix == 10 && value < 0;
	Unsigned magnitude = negative ? static_cast<Unsigned>(0) - static_cast<Unsigned>(value) : static_cast<Unsigned>(value);
	char reversed[72];
	int count = 0;
	do
	{
		reversed[count++] = digits[magnitude % static_cast<Unsigned>(radix)];
		magnitude /= static_cast<Unsigned>(radix);
	} while (magnitude);
	char* out = buffer;
	if (negative)
	{
		*out++ = '-';
	}
	while (count)
	{
		*out++ = reversed[--count];
	}
	*out = '\0';
	return buffer;
}
}

inline std::uint64_t GetTickCount64()
{
	return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now().time_since_epoch()).count());
}

inline DWORD GetTickCount()
{
	return static_cast<DWORD>(GetTickCount64());
}

inline DWORD timeGetTime()
{
	return GetTickCount();
}

inline void Sleep(DWORD milliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

inline DWORD GetLastError()
{
	return static_cast<DWORD>(errno);
}

inline void GetLocalTime(SYSTEMTIME* time)
{
	portability::CurrentTime(time, localtime_r);
}

inline void GetSystemTime(SYSTEMTIME* time)
{
	portability::CurrentTime(time, gmtime_r);
}

inline int stricmp(const char* left, const char* right) { return strcasecmp(left, right); }
inline int _stricmp(const char* left, const char* right) { return strcasecmp(left, right); }

inline errno_t strcpy_s(char* destination, std::size_t size, const char* source)
{
	if (!destination || size == 0 || !source)
	{
		return EINVAL;
	}
	const std::size_t length = std::strlen(source);
	if (length >= size)
	{
		destination[0] = '\0';
		return ERANGE;
	}
	std::memcpy(destination, source, length + 1);
	return 0;
}

template <std::size_t Size>
errno_t strcpy_s(char (&destination)[Size], const char* source)
{
	return strcpy_s(destination, Size, source);
}

inline errno_t strncpy_s(char* destination, std::size_t size, const char* source, std::size_t count)
{
	if (!destination || size == 0 || !source)
	{
		return EINVAL;
	}
	std::size_t length = strnlen(source, count);
	errno_t result = 0;
	if (length >= size)
	{
		if (count != _TRUNCATE)
		{
			destination[0] = '\0';
			return ERANGE;
		}
		length = size - 1;
		result = STRUNCATE;
	}
	std::memcpy(destination, source, length);
	destination[length] = '\0';
	return result;
}

template <std::size_t Size>
errno_t strncpy_s(char (&destination)[Size], const char* source, std::size_t count)
{
	return strncpy_s(destination, Size, source, count);
}

inline errno_t strcat_s(char* destination, std::size_t size, const char* source)
{
	if (!destination || size == 0 || !source)
	{
		return EINVAL;
	}
	const std::size_t used = strnlen(destination, size);
	if (used == size)
	{
		destination[0] = '\0';
		return EINVAL;
	}
	return strcpy_s(destination + used, size - used, source);
}

template <std::size_t Size>
errno_t strcat_s(char (&destination)[Size], const char* source)
{
	return strcat_s(destination, Size, source);
}

inline errno_t strncat_s(char* destination, std::size_t size, const char* source, std::size_t count)
{
	if (!destination || size == 0 || !source)
	{
		return EINVAL;
	}
	const std::size_t used = strnlen(destination, size);
	if (used == size)
	{
		destination[0] = '\0';
		return EINVAL;
	}
	return strncpy_s(destination + used, size - used, source, count);
}

template <std::size_t Size>
errno_t strncat_s(char (&destination)[Size], const char* source, std::size_t count)
{
	return strncat_s(destination, Size, source, count);
}

inline int _vsnprintf(char* buffer, std::size_t count, const char* format, va_list args)
{
	const int length = std::vsnprintf(buffer, count, format, args);
	return (length < 0 || static_cast<std::size_t>(length) > count) ? -1 : length;
}

inline int _snprintf(char* buffer, std::size_t count, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	const int length = _vsnprintf(buffer, count, format, args);
	va_end(args);
	return length;
}

inline int _vsnprintf_s(char* buffer, std::size_t size, std::size_t count, const char* format, va_list args)
{
	const std::size_t limit = (count == _TRUNCATE || count >= size) ? size : count + 1;
	return portability::FormatTruncated(buffer, limit, format, args);
}

inline int _snprintf_s(char* buffer, std::size_t size, std::size_t count, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	const int length = _vsnprintf_s(buffer, size, count, format, args);
	va_end(args);
	return length;
}

template <std::size_t Size>
int _snprintf_s(char (&buffer)[Size], std::size_t count, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	const int length = _vsnprintf_s(buffer, Size, count, format, args);
	va_end(args);
	return length;
}

inline char* itoa(int value, char* buffer, int radix) { return portability::ToString(value, buffer, radix); }
inline char* _itoa(int value, char* buffer, int radix) { return portability::ToString(value, buffer, radix); }
inline char* _i64toa(long long value, char* buffer, int radix) { return portability::ToString(value, buffer, radix); }
inline long long _atoi64(const char* text) { return std::strtoll(text, nullptr, 10); }

inline char* _strlwr(char* text)
{
	for (char* p = text; *p; ++p)
	{
		*p = static_cast<char>(std::tolower(static_cast<unsigned char>(*p)));
	}
	return text;
}

inline char* _strupr(char* text)
{
	for (char* p = text; *p; ++p)
	{
		*p = static_cast<char>(std::toupper(static_cast<unsigned char>(*p)));
	}
	return text;
}

inline char* strlwr(char* text) { return _strlwr(text); }
inline char* _strdup(const char* text) { return strdup(text); }

inline int _access(const char* path, int mode) { return access(path, mode); }
inline int _setmaxstdio(int count) { return count; }

inline int _ismbslead(const unsigned char*, const unsigned char*) { return 0; }

#define _T(text) text
#define _TEXT(text) text
#define _tcsncpy std::strncpy
#define _tcscat std::strcat
#define _ftprintf std::fprintf

#endif
