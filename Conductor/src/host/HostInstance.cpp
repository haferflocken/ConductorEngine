#include <host/HostInstance.h>

#include <asset/AssetManager.h>
#include <behave/BehaveContext.h>
#include <behave/BehaviourTreeEvaluationSystem.h>
#include <client/ClientID.h>
#include <conductor/GameData.h>
#include <input/InputSystem.h>
#include <mesh/SkeletonSystem.h>
#include <mesh/SkeletonMatrixCollectionSystem.h>
#include <scene/RelativeTransformSystem.h>
#include <scene/SceneAnchorSystem.h>
#include <scene/UnboundedScene.h>

namespace Internal_HostInstance
{
constexpr const char* k_chunkSourceDirectory = "scenes/test_scene";
constexpr const char* k_chunkUserDirectory = "";
}

namespace Host
{
HostInstance::HostInstance(const Conductor::GameData& gameData)
	// Entities and components the host creates begin their IDs at 0.
	: m_entityManager(gameData.GetAssetManager(), gameData.GetComponentReflector(), ECS::EntityID(0), 0)
	, m_ecsTransmitter()
	, m_inputSystem(m_entityManager.RegisterSystem(Mem::MakeUnique<Input::InputSystem>()))
{
	using namespace Internal_HostInstance;

	m_entityManager.RegisterSystem(Mem::MakeUnique<Behave::BehaviourTreeEvaluationSystem>(Behave::BehaveContext{
		gameData.GetBehaveASTInterpreter(),
		m_entityManager }));

	Scene::UnboundedScene& scene = m_entityManager.RegisterSystem(Mem::MakeUnique<Scene::UnboundedScene>(
		gameData.GetDataDirectory() / k_chunkSourceDirectory,
		gameData.GetUserDirectory() / k_chunkUserDirectory));

	m_entityManager.RegisterSystem(Mem::MakeUnique<Scene::SceneAnchorSystem>(scene));

	// SkeletonSystem produces an entity hierarchy which should happen before RelativeTransformSystem runs.
	m_entityManager.RegisterSystem(Mem::MakeUnique<Mesh::SkeletonSystem>());
	m_entityManager.RegisterSystem(Mem::MakeUnique<Scene::RelativeTransformSystem>());
	// SkeletonMatrixCollectionSystem depends on the output of RelativeTransformSystem.
	m_entityManager.RegisterSystem(Mem::MakeUnique<Mesh::SkeletonMatrixCollectionSystem>());
}

void HostInstance::NotifyOfClientConnected(const Client::ClientID clientID, const Input::InputStateManager& inputStateManager)
{
	m_ecsTransmitter.NotifyOfClientConnected(clientID);
	m_inputSystem.AddClient(clientID, inputStateManager);
}

void HostInstance::NotifyOfClientDisconnected(const Client::ClientID clientID)
{
	m_inputSystem.RemoveClient(clientID);
	m_ecsTransmitter.NotifyOfClientDisconnected(clientID);
}

void HostInstance::NotifyOfFrameAcknowledgement(const Client::ClientID clientID, const uint64_t frameIndex)
{
	m_ecsTransmitter.NotifyOfFrameAcknowledgement(clientID, frameIndex);
}

void HostInstance::StoreECSFrame()
{
	ECS::SerializedEntitiesAndComponents serializedFrame;
	m_entityManager.FullySerializeAllEntitiesAndComponentsMatchingFilter(
		[](const ECS::Entity& entity)
		{
			return (entity.GetFlags() & ECS::EntityFlags::Networked) != ECS::EntityFlags::None;
		},
		serializedFrame);

	m_ecsTransmitter.AddSerializedFrame(std::move(serializedFrame));
}

void HostInstance::SerializeECSUpdateTransmission(
	const Client::ClientID clientID,
	Collection::Vector<uint8_t>& outTransmission)
{
	m_ecsTransmitter.TransmitFrame(clientID, outTransmission);
}

void HostInstance::Update(const Unit::Time::Millisecond delta)
{
	// TODO
	m_entityManager.Update(delta);
}
}
