#include <islandgame/client/IslandGameClient.h>

#include <islandgame/IslandGameData.h>

#include <behave/BehaveContext.h>
#include <behave/BehaviourTreeEvaluationSystem.h>
#include <condui/Condui.h>
#include <condui/TextInputComponent.h>
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
		isCameraCreated = true;

		m_entityManager.CreateEntity(
			*m_gameData.GetEntityInfoManager().FindEntityInfo(Util::CalcHash("player.json")));

		// Create a camera looking at the center of the scene.
		ECS::Entity& cameraEntity = m_entityManager.CreateEntity(
			*m_gameData.GetEntityInfoManager().FindEntityInfo(Util::CalcHash("camera.json")));

		auto& cameraTransformComponent = *m_entityManager.FindComponent<Scene::SceneTransformComponent>(cameraEntity);

		const Math::Matrix4x4 cameraTranslation = Math::Matrix4x4::MakeTranslation(Math::Vector3(0.0f, 0.0f, -5.0f));

		cameraTransformComponent.m_modelToWorldMatrix = cameraTranslation;

		// Create a console and attach it to the camera.
		Condui::ElementRoot consoleRoot;
		consoleRoot.m_uiTransform.SetTranslation(Math::Vector3(0.0f, 0.0f, 0.0f));
		consoleRoot.m_element = Condui::MakeTextInputElement(0.5f, 0.025f,
			[this](Condui::TextInputComponent& component, const char* text)
			{
				if (strcmp(text, "\r") == 0)
				{
					// TODO useful commands
					if (strcmp(component.m_text.c_str(), "yellow") == 0)
					{
						component.m_backgroundColour = Image::ColoursARBG::k_yellow;
					}
					else if (strcmp(component.m_text.c_str(), "cyan") == 0)
					{
						component.m_backgroundColour = Image::ColoursARBG::k_cyan;
					}
					component.m_text.clear();
				}
				else
				{
					Condui::TextInputComponent::DefaultInputHandler(component, text);
				}
			}, 1.0f);

		ECS::Entity& consoleEntity =
			Condui::CreateConduiRootEntity(m_gameData.GetEntityInfoManager(), m_entityManager, consoleRoot);
		m_entityManager.SetParentEntity(consoleEntity, &cameraEntity);

		auto& consoleTransformComponent = *m_entityManager.FindComponent<Scene::SceneTransformComponent>(consoleEntity);
		consoleTransformComponent.m_childToParentMatrix.SetTranslation(Math::Vector3(-0.25f, -0.25f, 0.5f));
	}

	m_entityManager.Update(delta);
}
