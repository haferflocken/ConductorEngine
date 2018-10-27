#include <client/IClient.h>

#include <client/InputMessage.h>

namespace Client
{
void IClient::NotifyOfECSUpdateTransmission(const Collection::Vector<uint8_t>& transmissionBytes)
{
	m_entityManager.ApplyDeltaTransmission(transmissionBytes);
}

void IClient::NotifyOfInputMessage(const InputMessage& message)
{
	const uint64_t messageTypeBit = 1ui64 << static_cast<uint64_t>(message.GetTag());

	for (auto&& entry : m_inputCallbacks)
	{
		InputCallback& callback = entry.second;
		if ((callback.m_inputTypeMask & messageTypeBit) != 0)
		{
			callback.m_handler(message);
		}
	}
}

uint64_t IClient::RegisterInputCallback(uint64_t inputTypeMask, std::function<void(const InputMessage)>&& callbackFn)
{
	const uint64_t id = m_nextCallbackID;
	++m_nextCallbackID;

	InputCallback& callback = m_inputCallbacks[id];
	callback.m_inputTypeMask = inputTypeMask;
	callback.m_handler = std::move(callbackFn);

	return id;
}

void IClient::UnregisterInputCallback(const uint64_t callbackID)
{
	[[maybe_unused]] const bool isRemoved = m_inputCallbacks.TryRemove(callbackID);
	AMP_ASSERT(isRemoved, "Failed to remove client input callback for ID %llu.", callbackID);
}
}
