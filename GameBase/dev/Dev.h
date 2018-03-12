#pragma once

#include <cstdio>
#include <stdexcept>

class Dev
{
public:
	template <typename... Args>
	static void Assert(const bool check, const char* const format, const Args&... args);

	template <typename... Args>
	static void FatalAssert(const bool check, const char* const format, const Args&... args);

	template <typename... Args>
	static void Log(const char* const format, const Args&... args);

	template <typename... Args>
	static void LogWarning(const char* const format, const Args&... args);

	template <typename... Args>
	static void LogError(const char* const format, const Args&... args);

	template <typename... Args>
	static void FatalError(const char* const format, const Args&... args);

private:
	static constexpr size_t sk_bufferSize = 512;

	static void PrintMessage(const char* const message);
};

template <typename... Args>
inline void Dev::Assert(const bool check, const char* const format, const Args&... args)
{
#ifdef _DEBUG
	if (!check)
	{
		LogError(format, args...);
	}
#endif
}

template <typename... Args>
inline void Dev::FatalAssert(const bool check, const char* const format, const Args&... args)
{
#ifdef _DEBUG
	if (!check)
	{
		FatalError(format, args...);
	}
#endif
}

template <typename... Args>
inline void Dev::Log(const char* const format, const Args&... args)
{
	char buffer[sk_bufferSize];
	std::snprintf(buffer, sk_bufferSize, format, args...);
	PrintMessage(buffer);
}

template <typename... Args>
inline void Dev::LogWarning(const char* const format, const Args&... args)
{
	char buffer[sk_bufferSize];
	std::snprintf(buffer, sk_bufferSize, format, args...);
	PrintMessage(buffer);
}

template <typename... Args>
inline void Dev::LogError(const char* const format, const Args&... args)
{
	char buffer[sk_bufferSize];
	std::snprintf(buffer, sk_bufferSize, format, args...);
	PrintMessage(buffer);
}

template <typename... Args>
inline void Dev::FatalError(const char* const format, const Args&... args)
{
	LogError(format, args...);
#ifdef _DEBUG
	throw std::runtime_error("FATAL ERROR");
#else
	std::terminate();
#endif
}
