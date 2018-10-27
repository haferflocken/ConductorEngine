#pragma once

#include <collection/VectorMap.h>
#include <input/InputMessage.h>

#include <cstdint>
#include <functional>

namespace Input
{
/**
 * Owns callbacks for input messages and dispatches messages to them.
 */
class CallbackRegistry final
{
public:
	void NotifyOfInputMessage(const Input::InputMessage& message);

	template <typename... AcceptedTypes>
	uint64_t RegisterInputCallback(std::function<void(const Input::InputMessage)>&& callbackFn);

	void UnregisterInputCallback(const uint64_t callbackID);

private:
	uint64_t RegisterInputCallback(uint64_t inputTypeMask, std::function<void(const InputMessage)>&& callbackFn);

	// Encapsulates a callback function that is only called for certain InputMessage types.
	struct InputCallback final
	{
		uint64_t m_inputTypeMask{ 0 };
		std::function<void(const InputMessage&)> m_handler{};
	};

	uint64_t m_nextCallbackID{ 0 };
	Collection::VectorMap<uint64_t, InputCallback> m_inputCallbacks;
};
}

// Inline implementations.
namespace Input
{
template <typename... AcceptedTypes>
uint64_t CallbackRegistry::RegisterInputCallback(std::function<void(const InputMessage)>&& callbackFn)
{
	if constexpr (sizeof...(AcceptedTypes) == 0)
	{
		return RegisterInputCallback(UINT64_MAX, std::move(callbackFn));
	}
	else
	{
		uint64_t mask = 0;
		for (size_t tag : { InputMessage::TagFor<AcceptedTypes>()... })
		{
			mask |= (1ui64 << tag);
		}
		return RegisterInputCallback(mask, std::move(callbackFn));
	}
}
}
