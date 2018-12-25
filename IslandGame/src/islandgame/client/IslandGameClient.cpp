#include <islandgame/client/IslandGameClient.h>

#include <islandgame/IslandGameData.h>

#include <behave/BehaveContext.h>
#include <behave/BehaviourTreeEvaluationSystem.h>
#include <condui/Condui.h>
#include <ecs/EntityInfoManager.h>
#include <scene/RelativeTransformSystem.h>
#include <scene/SceneTransformComponent.h>
#include <scene/SceneTransformComponentInfo.h>

IslandGame::Client::IslandGameClient::IslandGameClient(
	const IslandGameData& gameData, ::Client::ConnectedHost& connectedHost)
	: IClient(gameData.GetAssetManager(), gameData.GetComponentReflector(), connectedHost)
	, m_gameData(gameData)
{
	m_inputStateManager.SetInputName({ Input::InputSource::k_mouseID, Input::InputSource::k_mouseAxisX },
		Util::CalcHash("mouse_x"));
	m_inputStateManager.SetInputName({ Input::InputSource::k_mouseID, Input::InputSource::k_mouseAxisY },
		Util::CalcHash("mouse_y"));

	const Behave::BehaveContext context{
		m_gameData.GetBehaviourTreeManager(),
		m_gameData.GetBehaveASTInterpreter(),
		m_entityManager };
	m_entityManager.RegisterSystem(Mem::MakeUnique<Behave::BehaviourTreeEvaluationSystem>(context));

	m_entityManager.RegisterSystem(Mem::MakeUnique<Scene::RelativeTransformSystem>());
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

		Condui::ElementRoot elementRoot;
		elementRoot.m_uiTransform.SetTranslation(Math::Vector3(0.0f, 0.0f, 0.0f));
		elementRoot.m_element = Condui::MakeTextInputElement(1.0f, 1.0f);

		Condui::CreateConduiRootEntity(m_gameData.GetEntityInfoManager(), m_entityManager, elementRoot);

		isCameraCreated = true;
	}

	m_entityManager.Update(delta);
}
