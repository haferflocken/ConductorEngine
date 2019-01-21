#include <conductor/IGameData.h>

#include <asset/AssetManager.h>
#include <behave/ast/Interpreter.h>
#include <behave/BehaviourForest.h>
#include <behave/BehaviourNodeFactory.h>
#include <behave/BehaviourTreeComponent.h>
#include <behave/BehaviourTreeComponentInfo.h>
#include <behave/BlackboardComponent.h>
#include <behave/BlackboardComponentInfo.h>
#include <ecs/ComponentInfoFactory.h>
#include <ecs/ComponentReflector.h>
#include <ecs/EntityInfoManager.h>
#include <image/Pixel1Image.h>
#include <input/InputComponent.h>
#include <mesh/MeshComponent.h>
#include <mesh/MeshComponentInfo.h>
#include <scene/AnchorComponent.h>
#include <scene/AnchorComponentInfo.h>
#include <scene/SceneSaveComponent.h>
#include <scene/SceneTransformComponent.h>
#include <scene/SceneTransformComponentInfo.h>

namespace Conductor
{
IGameData::IGameData(const File::Path& dataDirectory, const File::Path& userDirectory,
	Asset::AssetManager& assetManager)
	: m_dataDirectory(dataDirectory)
	, m_userDirectory(userDirectory)
	, m_assetManager(assetManager)
	, m_componentReflector(Mem::MakeUnique<ECS::ComponentReflector>())
	, m_behaveASTInterpreter(Mem::MakeUnique<Behave::AST::Interpreter>(*m_componentReflector))
	, m_behaviourNodeFactory(Mem::MakeUnique<Behave::BehaviourNodeFactory>(*m_behaveASTInterpreter))
	, m_componentInfoFactory(Mem::MakeUnique<ECS::ComponentInfoFactory>())
	, m_entityInfoManager(Mem::MakeUnique<ECS::EntityInfoManager>(assetManager, *m_componentInfoFactory))
{
	// Register asset types.
	m_assetManager.RegisterAssetType<Image::Pixel1Image>(&Image::Pixel1Image::TryLoad);
	m_assetManager.RegisterAssetType<Mesh::StaticMesh>(&Mesh::StaticMesh::TryLoad);
	m_assetManager.RegisterAssetType<Behave::BehaviourForest>(
		[&](const File::Path& filePath, Behave::BehaviourForest* destination)
		{
			return Behave::BehaviourForest::TryLoad(*m_behaviourNodeFactory, filePath, destination);
		});

	// Register ECS component types.
	m_componentReflector->RegisterComponentType<Behave::BehaviourTreeComponent>();
	m_componentReflector->RegisterComponentType<Behave::BlackboardComponent>();
	m_componentReflector->RegisterComponentType<Input::InputComponent>();
	m_componentReflector->RegisterComponentType<Mesh::MeshComponent>();
	m_componentReflector->RegisterTagComponentType<Scene::SceneSaveComponent>();
	m_componentReflector->RegisterComponentType<Scene::SceneTransformComponent>();
	m_componentReflector->RegisterComponentType<Scene::AnchorComponent>();

	// Bind functions to the Behave interpreter.
	Behave::BlackboardComponent::BindFunctions(*m_behaveASTInterpreter);

	// Register ECS component info types.
	m_componentInfoFactory->RegisterFactoryFunction<Behave::BehaviourTreeComponentInfo>();
	m_componentInfoFactory->RegisterFactoryFunction<Behave::BlackboardComponentInfo>();
	m_componentInfoFactory->RegisterFactoryFunction<Input::InputComponentInfo>();
	m_componentInfoFactory->RegisterFactoryFunction<Mesh::MeshComponentInfo>();
	m_componentInfoFactory->RegisterFactoryFunction<Scene::SceneSaveComponentInfo>();
	m_componentInfoFactory->RegisterFactoryFunction<Scene::SceneTransformComponentInfo>();
	m_componentInfoFactory->RegisterFactoryFunction<Scene::AnchorComponentInfo>();
}

IGameData::~IGameData()
{
	// Unregister asset types in the opposite order in which they where registered.
	m_assetManager.UnregisterAssetType<Behave::BehaviourForest>();
	m_assetManager.UnregisterAssetType<Mesh::StaticMesh>();
	m_assetManager.UnregisterAssetType<Image::Pixel1Image>();
}

void IGameData::LoadEntityInfosInDirectory(const File::Path& directory)
{
	m_entityInfoManager->LoadEntityInfosInDirectory(directory);
}
}
