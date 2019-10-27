#include <client/ClientInstance.h>

#include <asset/AssetManager.h>
#include <behave/BehaveContext.h>
#include <behave/BehaviourTreeEvaluationSystem.h>
#include <client/ConnectedHost.h>
#include <conductor/GameData.h>
#include <input/InputMessage.h>
#include <input/InputSystem.h>
#include <mesh/SkeletonMatrixCollectionSystem.h>
#include <scene/RelativeTransformSystem.h>

namespace Client
{
ClientInstance::ClientInstance(const Conductor::GameData& gameData, ConnectedHost& connectedHost)
	: m_connectedHost(connectedHost)
	, m_inputStateManager(m_inputCallbackRegistry)
	// Entities and components the client creates have their high bit set.
	, m_entityManager(gameData.GetAssetManager(), gameData.GetComponentReflector(), ECS::EntityID(1ui32 << 31), 1ui64 << 63)
	, m_ecsReceiver()
{
	// The InputSystem is present for all clients.
	Input::InputSystem& inputSystem = m_entityManager.RegisterSystem(Mem::MakeUnique<Input::InputSystem>());
	inputSystem.AddClient(m_connectedHost.GetClientID(), m_inputStateManager);

	m_inputStateManager.SetInputName({ Input::InputSource::k_mouseID, Input::InputSource::k_mouseAxisX },
		Util::CalcHash("mouse_x"));
	m_inputStateManager.SetInputName({ Input::InputSource::k_mouseID, Input::InputSource::k_mouseAxisY },
		Util::CalcHash("mouse_y"));

	const Behave::BehaveContext context{
		gameData.GetBehaveASTInterpreter(),
		m_entityManager };
	m_entityManager.RegisterSystem(Mem::MakeUnique<Behave::BehaviourTreeEvaluationSystem>(context));

	m_entityManager.RegisterSystem(Mem::MakeUnique<Scene::RelativeTransformSystem>());
	// SkeletonMatrixCollectionSystem depends on the output of RelativeTransformSystem.
	m_entityManager.RegisterSystem(Mem::MakeUnique<Mesh::SkeletonMatrixCollectionSystem>());
}

void ClientInstance::NotifyOfECSUpdateTransmission(const Collection::Vector<uint8_t>& transmissionBytes)
{
	const ECS::SerializedEntitiesAndComponents* const newFrame =
		m_ecsReceiver.TryReceiveFrameTransmission(transmissionBytes.GetConstView());
	if (newFrame != nullptr)
	{
		m_entityManager.SetNetworkedEntitiesToFullSerialization(*newFrame);
		m_connectedHost.TransmitFrameAcknowledgement(m_ecsReceiver.GetLastSeenFrameIndex());
	}
}

void ClientInstance::NotifyOfInputMessage(const Input::InputMessage& message)
{
	m_inputCallbackRegistry.NotifyOfInputMessage(message);
}

Collection::Vector<uint8_t> ClientInstance::SerializeInputStateTransmission() const
{
	return m_inputStateManager.SerializeFullTransmission();
}

void ClientInstance::Update(const Unit::Time::Millisecond delta)
{
	// TODO
	m_entityManager.Update(delta);
}

void ClientInstance::PostUpdate()
{
	m_inputStateManager.ResetInputStates();
}
}
