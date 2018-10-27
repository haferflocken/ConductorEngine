#pragma once

#include <collection/Vector.h>
#include <collection/VectorMap.h>
#include <ecs/EntityManager.h>

#include <functional>

namespace Client
{
class ConnectedHost;
struct InputMessage;
enum class InputMessageType : uint8_t;

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
	void NotifyOfInputMessage(const Client::InputMessage& message);

	// Register an input callback.
	uint64_t RegisterInputCallback(std::function<void(const Client::InputMessage)>&& callbackFn);
	uint64_t RegisterInputCallback(const Collection::ArrayView<InputMessageType>& acceptedTypes,
		std::function<void(const Client::InputMessage)>&& callbackFn);
	void UnregisterInputCallback(const uint64_t callbackID);

	virtual void Update(const Unit::Time::Millisecond delta) = 0;

private:
	// Encapsulates a callback function that is only called for certain InputMessage types.
	struct InputCallback final
	{
		uint64_t m_inputTypeMask{ 0 };
		std::function<void(const Client::InputMessage&)> m_handler{};
	};

	uint64_t m_nextCallbackID{ 0 };
	Collection::VectorMap<uint64_t, InputCallback> m_inputCallbacks;
};
}
