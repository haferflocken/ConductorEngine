#pragma once

#include <cstdio>
#include <stdexcept>

#pragma intrinsic(__debugbreak)

#ifdef _DEBUG
#define AMP_ASSERTS_ENABLED 1
#else
#define AMP_ASSERTS_ENABLED 0
#endif

#if AMP_ASSERTS_ENABLED == 1

#define AMP_ASSERT(CHECK, FORMAT, ...) \
	do { \
		if (!(CHECK)) {\
			Dev::LogError(FORMAT, __VA_ARGS__); \
			__debugbreak(); \
		} \
	} while(false)

#define AMP_FATAL_ASSERT(CHECK, FORMAT, ...) \
	do { \
		if (!(CHECK)) { \
			Dev::FatalError(FORMAT, __VA_ARGS__); \
		} \
	} while(false)

#else
#define AMP_ASSERT(...)
#define AMP_FATAL_ASSERT(...)
#endif

class Dev
{
public:
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
#if AMP_ASSERTS_ENABLED == 1
	__debugbreak();
#endif
	std::terminate();
}
