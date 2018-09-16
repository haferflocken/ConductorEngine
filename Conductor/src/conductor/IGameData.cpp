#include <conductor/IGameData.h>

#include <behave/ast/Interpreter.h>
#include <behave/BehaviourNodeFactory.h>
#include <behave/BehaviourTreeComponent.h>
#include <behave/BehaviourTreeComponentInfo.h>
#include <behave/BehaviourTreeManager.h>
#include <behave/BlackboardComponent.h>
#include <behave/BlackboardComponentInfo.h>
#include <ecs/ComponentInfoFactory.h>
#include <ecs/ComponentReflector.h>
#include <ecs/EntityInfoManager.h>
#include <scene/SceneTransformComponent.h>
#include <scene/SceneTransformComponentInfo.h>

namespace Conductor
{
IGameData::IGameData(Asset::AssetManager& assetManager)
	: m_assetManager(assetManager)
	, m_componentReflector(Mem::MakeUnique<ECS::ComponentReflector>())
	, m_behaveASTInterpreter(Mem::MakeUnique<Behave::AST::Interpreter>(*m_componentReflector))
	, m_behaviourNodeFactory(Mem::MakeUnique<Behave::BehaviourNodeFactory>(*m_behaveASTInterpreter))
	, m_behaviourTreeManager(Mem::MakeUnique<Behave::BehaviourTreeManager>(*m_behaviourNodeFactory))
	, m_componentInfoFactory(Mem::MakeUnique<ECS::ComponentInfoFactory>())
	, m_entityInfoManager(Mem::MakeUnique<ECS::EntityInfoManager>(*m_componentInfoFactory, *m_behaviourTreeManager))
{
	// Register ECS component types.
	m_componentReflector->RegisterComponentType<Behave::BehaviourTreeComponent>();
	m_componentReflector->RegisterComponentType<Behave::BlackboardComponent>();
	m_componentReflector->RegisterComponentType<Scene::SceneTransformComponent>();

	// Bind functions to the Behave interpreter.
	Behave::BlackboardComponent::BindFunctions(*m_behaveASTInterpreter);

	// Register ECS component info types.
	m_componentInfoFactory->RegisterFactoryFunction<Behave::BehaviourTreeComponentInfo>();
	m_componentInfoFactory->RegisterFactoryFunction<Behave::BlackboardComponentInfo>();
	m_componentInfoFactory->RegisterFactoryFunction<Scene::SceneTransformComponentInfo>();
}

IGameData::~IGameData()
{}

void IGameData::LoadBehaviourTreesInDirectory(const File::Path& directory)
{
	m_behaviourTreeManager->LoadTreesInDirectory(directory);
}

void IGameData::LoadEntityInfosInDirectory(const File::Path& directory)
{
	m_entityInfoManager->LoadEntityInfosInDirectory(directory);
}
}
