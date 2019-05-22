#pragma once

#include <cstdio>
#include <stdexcept>

#pragma intrinsic(__debugbreak)

#ifdef _DEBUG
#define AMP_ASSERTS_ENABLED 1
#else
#define AMP_ASSERTS_ENABLED 0
#endif

#define AMP_LOG_BUFFER_SIZE 512

#define AMP_LOG(FORMAT, ...) \
	do {\
		char buffer[AMP_LOG_BUFFER_SIZE]; \
		_snprintf_s(buffer, AMP_LOG_BUFFER_SIZE, FORMAT, __VA_ARGS__); \
		Dev::PrintMessage(Dev::MessageType::Info, buffer); \
	} while(false)

#define AMP_LOG_WARNING(FORMAT, ...) \
	do {\
		char buffer[AMP_LOG_BUFFER_SIZE]; \
		_snprintf_s(buffer, AMP_LOG_BUFFER_SIZE, FORMAT, __VA_ARGS__); \
		Dev::PrintMessage(Dev::MessageType::Warning, buffer); \
	} while(false)

#define AMP_LOG_ERROR(FORMAT, ...) \
	do {\
		char buffer[AMP_LOG_BUFFER_SIZE]; \
		_snprintf_s(buffer, AMP_LOG_BUFFER_SIZE, FORMAT, __VA_ARGS__); \
		Dev::PrintMessage(Dev::MessageType::Error, buffer); \
	} while(false)

#define AMP_FATAL_ERROR(FORMAT, ...) \
	do {\
		char buffer[AMP_LOG_BUFFER_SIZE]; \
		_snprintf_s(buffer, AMP_LOG_BUFFER_SIZE, FORMAT, __VA_ARGS__); \
		Dev::PrintMessage(Dev::MessageType::FatalError, buffer); \
		__debugbreak(); \
		std::terminate(); \
	} while(false)

#if AMP_ASSERTS_ENABLED == 1

#define AMP_ASSERT(CHECK, FORMAT, ...) \
	do { \
		if (!(CHECK)) {\
			AMP_LOG_ERROR(FORMAT, __VA_ARGS__); \
			__debugbreak(); \
		} \
	} while(false)

#define AMP_FATAL_ASSERT(CHECK, FORMAT, ...) \
	do { \
		if (!(CHECK)) { \
			AMP_FATAL_ERROR(FORMAT, __VA_ARGS__); \
		} \
	} while(false)

#else
#define AMP_ASSERT(...)
#define AMP_FATAL_ASSERT(...)
#endif

namespace Dev
{
	enum class MessageType
	{
		Info = 0,
		Warning,
		Error,
		FatalError,
		Count
	};

	void SetOutputFor(const MessageType messageType, std::ostream& ostream);
	void PrintMessage(const MessageType messageType, const char* const message);
};
