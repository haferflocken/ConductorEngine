#include <islandgame/host/IslandGameHost.h>

#include <islandgame/IslandGameData.h>

#include <asset/AssetManager.h>
#include <behave/BehaveContext.h>
#include <behave/BehaviourTreeEvaluationSystem.h>
#include <input/InputSystem.h>
#include <mesh/MeshComponent.h>
#include <mesh/SkeletonMatrixCollectionSystem.h>
#include <mesh/SkeletonSystem.h>
#include <scene/RelativeTransformSystem.h>
#include <scene/SceneAnchorSystem.h>
#include <scene/UnboundedScene.h>

namespace Internal_IslandGameHost
{
constexpr const char* k_chunkSourceDirectory = "scenes/test_scene";
constexpr const char* k_chunkUserDirectory = "";
}

IslandGame::Host::IslandGameHost::IslandGameHost(const IslandGameData& gameData)
	: IHost(gameData.GetAssetManager(), gameData.GetComponentReflector())
	, m_gameData(gameData)
{
	using namespace Internal_IslandGameHost;

	m_entityManager.RegisterSystem(Mem::MakeUnique<Behave::BehaviourTreeEvaluationSystem>(Behave::BehaveContext{
		m_gameData.GetBehaveASTInterpreter(),
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

IslandGame::Host::IslandGameHost::~IslandGameHost()
{
}

void IslandGame::Host::IslandGameHost::Update(const Unit::Time::Millisecond delta)
{
	static int s_spawnIndex = 0;
	if (s_spawnIndex < 16)
	{
		Asset::AssetManager& assetManager = m_gameData.GetAssetManager();

		const auto playerComponents = { Scene::SceneTransformComponent::k_type,
			Mesh::MeshComponent::k_type,
			Mesh::SkeletonRootComponent::k_type };

		ECS::Entity& player = m_entityManager.CreateEntityWithComponents(
			{ playerComponents.begin(), playerComponents.size() }, ECS::EntityFlags::Networked, ECS::EntityLayer());

		auto& meshComponent = *m_entityManager.FindComponent<Mesh::MeshComponent>(player);
		meshComponent.m_meshHandle =
			assetManager.RequestAsset<Mesh::TriangleMesh>(File::MakePath("meshes/offset-root-bone.fbx"));
		//assetManager.RequestAsset<Mesh::TriangleMesh>(File::MakePath("meshes/cubes-v6.cms"));

		auto& sceneTransformComponent = *m_entityManager.FindComponent<Scene::SceneTransformComponent>(player);
		sceneTransformComponent.m_modelToWorldMatrix = Math::Matrix4x4::MakeTranslation(0.0f, 0.0f, s_spawnIndex * 2.0f);
		++s_spawnIndex;
	}

	m_entityManager.Update(delta);
}
