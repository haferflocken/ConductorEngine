#pragma once

#include <collection/Vector.h>
#include <collection/VectorMap.h>
#include <ecs/EntityManager.h>

#include <functional>

namespace Input { struct InputMessage; }

namespace Client
{
class ConnectedHost;

// IClient is the interface a game's client must implement.
class IClient
{
protected:
	ConnectedHost& m_connectedHost;
	ECS::EntityManager m_entityManager;

public:
	IClient(Asset::AssetManager& assetManager, const ECS::ComponentReflector& componentReflector,
		ConnectedHost& connectedHost)
		: m_connectedHost(connectedHost)
		, m_entityManager(assetManager, componentReflector, false)
	{}

	ECS::EntityManager& GetEntityManager() { return m_entityManager; }
	const ECS::EntityManager& GetEntityManager() const { return m_entityManager; }

	void NotifyOfECSUpdateTransmission(const Collection::Vector<uint8_t>& transmissionBytes);
	void NotifyOfInputMessage(const Input::InputMessage& message);

	template <typename... AcceptedTypes>
	uint64_t RegisterInputCallback(std::function<void(const Input::InputMessage)>&& callbackFn);

	void UnregisterInputCallback(const uint64_t callbackID);

	virtual void Update(const Unit::Time::Millisecond delta) = 0;

private:
	uint64_t RegisterInputCallback(uint64_t inputTypeMask,
		std::function<void(const Input::InputMessage)>&& callbackFn);

	// Encapsulates a callback function that is only called for certain InputMessage types.
	struct InputCallback final
	{
		uint64_t m_inputTypeMask{ 0 };
		std::function<void(const Input::InputMessage&)> m_handler{};
	};

	uint64_t m_nextCallbackID{ 0 };
	Collection::VectorMap<uint64_t, InputCallback> m_inputCallbacks;
};
}

// Inline implementations.
namespace Client
{
template <typename... AcceptedTypes>
uint64_t IClient::RegisterInputCallback(std::function<void(const Input::InputMessage)>&& callbackFn)
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
