#include <islandgame/client/IslandGameClient.h>

#include <islandgame/IslandGameData.h>

#include <asset/AssetManager.h>
#include <behave/BehaveContext.h>
#include <behave/BehaviourTreeEvaluationSystem.h>
#include <condui/Condui.h>
#include <condui/EntityInspector.h>
#include <condui/TextInputComponent.h>
#include <input/InputComponent.h>
#include <mem/InspectorInfo.h>
#include <mesh/MeshComponent.h>
#include <mesh/SkeletonMatrixCollectionSystem.h>
#include <renderer/CameraComponent.h>
#include <scene/AnchorComponent.h>
#include <scene/RelativeTransformSystem.h>
#include <scene/SceneTransformComponent.h>

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
		m_gameData.GetBehaveASTInterpreter(),
		m_entityManager };
	m_entityManager.RegisterSystem(Mem::MakeUnique<Behave::BehaviourTreeEvaluationSystem>(context));

	m_entityManager.RegisterSystem(Mem::MakeUnique<Scene::RelativeTransformSystem>());
	// SkeletonMatrixCollectionSystem depends on the output of RelativeTransformSystem.
	m_entityManager.RegisterSystem(Mem::MakeUnique<Mesh::SkeletonMatrixCollectionSystem>());
}

void IslandGame::Client::IslandGameClient::Update(const Unit::Time::Millisecond delta)
{
	// TODO remove this
	static bool isCameraCreated = false;
	if (!isCameraCreated)
	{
		isCameraCreated = true;
		Asset::AssetManager& assetManager = m_gameData.GetAssetManager();

		const auto playerComponents = { Scene::SceneTransformComponent::k_type,
			Scene::AnchorComponent::k_type,
			Mesh::MeshComponent::k_type,
			Input::InputComponent::k_type };

		ECS::Entity& player = m_entityManager.CreateEntityWithComponents(
			{ playerComponents.begin(), playerComponents.size() });

		auto& meshComponent = *m_entityManager.FindComponent<Mesh::MeshComponent>(player);
		meshComponent.m_meshHandle =
			assetManager.RequestAsset<Mesh::TriangleMesh>(File::MakePath("meshes/quad_v4.cms"));

		// Create a camera looking at the center of the scene.
		const auto cameraComponents = { Scene::SceneTransformComponent::k_type, Renderer::CameraComponent::k_type };

		ECS::Entity& cameraEntity = m_entityManager.CreateEntityWithComponents(
			{ cameraComponents.begin(), cameraComponents.size() });

		auto& cameraTransformComponent = *m_entityManager.FindComponent<Scene::SceneTransformComponent>(cameraEntity);

		const Math::Matrix4x4 cameraTranslation = Math::Matrix4x4::MakeTranslation(0.0f, 0.0f, -5.0f);
		const Math::Matrix4x4 cameraRotation = Math::Matrix4x4::MakeRotateX(0.5f);

		cameraTransformComponent.m_modelToWorldMatrix = cameraRotation * cameraTranslation;

		// Create a console and attach it to the camera.
		Collection::VectorMap<const char*, std::function<void(Condui::TextInputComponent&)>> commandMap;
		commandMap["yellow"] = [](Condui::TextInputComponent& component)
		{
			component.m_backgroundColour = Image::ColoursARBG::k_yellow;
		};
		commandMap["cyan"] = [](Condui::TextInputComponent& component)
		{
			component.m_backgroundColour = Image::ColoursARBG::k_cyan;
		};

		Condui::ConduiElement consoleElement =
			Condui::MakeTextInputCommandElement(0.5f, 0.025f, std::move(commandMap), Image::ColoursARBG::k_cyan);

		const Condui::FontInfo fontInfo{
			assetManager.RequestAsset<Image::Pixel1Image>(File::MakePath("fonts/Codepage-437-monochome.bmp")),
			9,
			16,
			Image::ColoursARBG::k_black };

		ECS::Entity& consoleEntity = Condui::CreateConduiEntity(m_entityManager, std::move(consoleElement), &fontInfo);
		m_entityManager.SetParentEntity(consoleEntity, &cameraEntity);

		auto& consoleTransformComponent = *m_entityManager.FindComponent<Scene::SceneTransformComponent>(consoleEntity);
		consoleTransformComponent.m_childToParentMatrix.SetTranslation(-0.25f, -0.25f, 0.5f);

		// Create an inspector and attach it to the camera.
		Condui::ConduiElement inspectorElement = Condui::MakeEntityInspector(
			m_gameData.GetComponentReflector(),
			m_entityManager,
			player,
			0.5f,
			0.025f);

		ECS::Entity& inspectorEntity =
			Condui::CreateConduiEntity(m_entityManager, std::move(inspectorElement), &fontInfo);
		m_entityManager.SetParentEntity(inspectorEntity, &cameraEntity);

		auto& inspectorTransformComponent = *m_entityManager.FindComponent<Scene::SceneTransformComponent>(inspectorEntity);
		inspectorTransformComponent.m_childToParentMatrix.SetTranslation(-0.25f, 0.25f, 0.5f);
	}

	m_entityManager.Update(delta);
}
