#include <client/IClient.h>

#include <client/InputMessage.h>

namespace Client
{
void IClient::NotifyOfECSUpdateTransmission(const Collection::Vector<uint8_t>& transmissionBytes)
{
	m_entityManager.ApplyDeltaTransmission(transmissionBytes);
}

void IClient::NotifyOfInputMessage(const Client::InputMessage& message)
{
	const uint64_t messageTypeBit = 1ui64 << static_cast<uint64_t>(message.m_type);

	for (auto&& entry : m_inputCallbacks)
	{
		InputCallback& callback = entry.second;
		if ((callback.m_inputTypeMask & messageTypeBit) != 0)
		{
			callback.m_handler(message);
		}
	}
}

uint64_t IClient::RegisterInputCallback(std::function<void(const Client::InputMessage)>&& callbackFn)
{
	const uint64_t id = m_nextCallbackID;
	++m_nextCallbackID;

	InputCallback& callback = m_inputCallbacks[id];
	callback.m_inputTypeMask = UINT64_MAX;
	callback.m_handler = std::move(callbackFn);

	return id;
}

uint64_t IClient::RegisterInputCallback(const Collection::ArrayView<InputMessageType>& acceptedTypes,
	std::function<void(const Client::InputMessage)>&& callbackFn)
{
	static_assert((sizeof(InputCallback::m_inputTypeMask) * 8) >= static_cast<size_t>(InputMessageType::Count));

	const uint64_t id = m_nextCallbackID;
	++m_nextCallbackID;

	InputCallback& callback = m_inputCallbacks[id];
	for (auto&& type : acceptedTypes)
	{
		callback.m_inputTypeMask |= (1ui64 << static_cast<uint64_t>(type));
	}
	callback.m_handler = std::move(callbackFn);

	return id;
}

void IClient::UnregisterInputCallback(const uint64_t callbackID)
{
	[[maybe_unused]] const bool isRemoved = m_inputCallbacks.TryRemove(callbackID);
	AMP_ASSERT(isRemoved, "Failed to remove client input callback for ID %llu.", callbackID);
}
}
