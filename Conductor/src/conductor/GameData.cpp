#include <conductor/GameData.h>

#include <asset/AssetManager.h>
#include <behave/ast/Interpreter.h>
#include <behave/BehaviourForest.h>
#include <behave/BehaviourNodeFactory.h>
#include <behave/BehaviourTreeComponent.h>
#include <behave/BlackboardComponent.h>
#include <ecs/ComponentReflector.h>
#include <image/Pixel1Image.h>
#include <input/InputComponent.h>
#include <mesh/FBXImporter.h>
#include <mesh/MeshComponent.h>
#include <mesh/SkeletonRootComponent.h>
#include <scene/AnchorComponent.h>
#include <scene/SceneSaveComponent.h>
#include <scene/SceneTransformComponent.h>

namespace Conductor
{
GameData::GameData(const File::Path& dataDirectory, const File::Path& userDirectory,
	Asset::AssetManager& assetManager)
	: m_dataDirectory(dataDirectory)
	, m_userDirectory(userDirectory)
	, m_assetManager(assetManager)
	, m_componentReflector(Mem::MakeUnique<ECS::ComponentReflector>())
	, m_behaveASTInterpreter(Mem::MakeUnique<Behave::AST::Interpreter>(*m_componentReflector))
	, m_behaviourNodeFactory(Mem::MakeUnique<Behave::BehaviourNodeFactory>(*m_behaveASTInterpreter))
{
	// Register asset types.
	m_assetManager.RegisterAssetType<Image::Pixel1Image>(".bmp", &Image::Pixel1Image::TryLoad);
	m_assetManager.RegisterAssetType<Mesh::TriangleMesh>(".cms", &Mesh::TriangleMesh::TryLoad);
	m_assetManager.RegisterAssetType<Mesh::TriangleMesh>(".fbx", &Mesh::TryImportFBX);
	m_assetManager.RegisterAssetType<Behave::BehaviourForest>(".behave",
		[&](const File::Path& filePath, Behave::BehaviourForest* destination)
		{
			return Behave::BehaviourForest::TryLoad(*m_behaviourNodeFactory, filePath, destination);
		});

	// Register ECS component types.
	m_componentReflector->RegisterComponentType<Behave::BehaviourTreeComponent>();
	m_componentReflector->RegisterComponentType<Behave::BlackboardComponent>();
	m_componentReflector->RegisterComponentType<Input::InputComponent>();
	m_componentReflector->RegisterComponentType<Mesh::MeshComponent>();
	m_componentReflector->RegisterComponentType<Mesh::SkeletonRootComponent>();
	m_componentReflector->RegisterComponentType<Scene::SceneSaveComponent>();
	m_componentReflector->RegisterComponentType<Scene::SceneTransformComponent>();
	m_componentReflector->RegisterComponentType<Scene::AnchorComponent>();

	// Bind functions to the Behave interpreter.
	Behave::BlackboardComponent::BindFunctions(*m_behaveASTInterpreter);
}

GameData::~GameData()
{
	// Unregister asset types in the opposite order in which they where registered.
	m_assetManager.UnregisterAssetType<Behave::BehaviourForest>();
	m_assetManager.UnregisterAssetType<Mesh::TriangleMesh>();
	m_assetManager.UnregisterAssetType<Image::Pixel1Image>();
}
}
