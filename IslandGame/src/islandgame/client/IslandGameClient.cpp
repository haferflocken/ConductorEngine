#include <islandgame/client/IslandGameClient.h>

#include <islandgame/IslandGameData.h>

#include <behave/BehaveContext.h>
#include <behave/BehaviourTreeEvaluationSystem.h>
#include <ecs/EntityInfoManager.h>
#include <input/InputSystem.h>
#include <scene/SceneTransformComponent.h>
#include <scene/SceneTransformComponentInfo.h>

IslandGame::Client::IslandGameClient::IslandGameClient(
	const IslandGameData& gameData, ::Client::ConnectedHost& connectedHost)
	: IClient(gameData.GetAssetManager(), gameData.GetComponentReflector(), connectedHost)
	, m_gameData(gameData)
{
	m_entityManager.RegisterSystem(Mem::MakeUnique<Input::InputSystem>(*this));

	const Behave::BehaveContext context{
		m_gameData.GetBehaviourTreeManager(),
		m_gameData.GetBehaveASTInterpreter(),
		m_entityManager };
	m_entityManager.RegisterSystem(Mem::MakeUnique<Behave::BehaviourTreeEvaluationSystem>(context));
}

void IslandGame::Client::IslandGameClient::Update(const Unit::Time::Millisecond delta)
{
	// TODO remove this
	static bool isCameraCreated = false;
	if (!isCameraCreated)
	{
		m_entityManager.CreateEntity(
			*m_gameData.GetEntityInfoManager().FindEntityInfo(Util::CalcHash("player.json")));
		ECS::Entity& cameraEntity = m_entityManager.CreateEntity(
			*m_gameData.GetEntityInfoManager().FindEntityInfo(Util::CalcHash("camera.json")));

		auto& sceneTransformComponent = *m_entityManager.FindComponent<Scene::SceneTransformComponent>(cameraEntity);
		sceneTransformComponent.m_matrix.SetTranslation(Math::Vector3(0.0f, 0.0f, -5.0f));

		isCameraCreated = true;
	}

	m_entityManager.Update(delta);
}
