#pragma once

#include <conductor/IGameData.h>
#include <file/Path.h>
#include <mem/UniquePtr.h>

namespace Behave
{
class BehaviourNodeFactory;
class BehaviourTreeManager;
}

namespace Behave::AST
{
class Interpreter;
}

namespace ECS
{
class ComponentInfoFactory;
class ComponentReflector;
class EntityInfoManager;
}

namespace IslandGame
{
/**
 * Holds data that will not change over the course of the game.
 */
class IslandGameData final : public Conductor::IGameData
{
public:
	IslandGameData();
	virtual ~IslandGameData();

	const Behave::AST::Interpreter& GetBehaveASTInterpreter() const { return *m_behaveASTInterpreter; }
	const Behave::BehaviourNodeFactory& GetBehaviourNodeFactory() const { return *m_behaviourNodeFactory; }
	const Behave::BehaviourTreeManager& GetBehaviourTreeManager() const { return *m_behaviourTreeManager; }
	
	const ECS::ComponentInfoFactory& GetComponentInfoFactory() const { return *m_componentInfoFactory; }
	const ECS::EntityInfoManager& GetEntityInfoManager() const { return *m_entityInfoManager; }
	
	const ECS::ComponentReflector& GetComponentReflector() const { return *m_componentReflector; }

	void LoadBehaviourTreesInDirectory(const File::Path& directory);
	void LoadEntityInfosInDirectory(const File::Path& directory);

private:
	Mem::UniquePtr<Behave::AST::Interpreter> m_behaveASTInterpreter;
	Mem::UniquePtr<Behave::BehaviourNodeFactory> m_behaviourNodeFactory;
	Mem::UniquePtr<Behave::BehaviourTreeManager> m_behaviourTreeManager;

	Mem::UniquePtr<ECS::ComponentInfoFactory> m_componentInfoFactory;
	Mem::UniquePtr<ECS::EntityInfoManager> m_entityInfoManager;

	Mem::UniquePtr<ECS::ComponentReflector> m_componentReflector;
};
}
