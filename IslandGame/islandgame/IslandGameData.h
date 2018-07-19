#pragma once

#include <conductor/IGameData.h>
#include <file/Path.h>
#include <mem/UniquePtr.h>

namespace Behave
{
class BehaviourNodeFactory;
class BehaviourTreeManager;
}

namespace ECS
{
class ActorComponentFactory;
class ActorComponentInfoFactory;
class ActorInfoManager;
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

	const Behave::BehaviourNodeFactory& GetBehaviourNodeFactory() const { return *m_behaviourNodeFactory; }
	const Behave::BehaviourTreeManager& GetBehaviourTreeManager() const { return *m_behaviourTreeManager; }
	
	const ECS::ActorComponentInfoFactory& GetActorComponentInfoFactory() const { return *m_actorComponentInfoFactory; }
	const ECS::ActorInfoManager& GetActorInfoManager() const { return *m_actorInfoManager; }
	
	const ECS::ActorComponentFactory& GetActorComponentFactory() const { return *m_actorComponentFactory; }

	void LoadBehaviourTreesInDirectory(const File::Path& directory);
	void LoadActorInfosInDirectory(const File::Path& directory);

private:
	Mem::UniquePtr<Behave::BehaviourNodeFactory> m_behaviourNodeFactory;
	Mem::UniquePtr<Behave::BehaviourTreeManager> m_behaviourTreeManager;

	Mem::UniquePtr<ECS::ActorComponentInfoFactory> m_actorComponentInfoFactory;
	Mem::UniquePtr<ECS::ActorInfoManager> m_actorInfoManager;

	Mem::UniquePtr<ECS::ActorComponentFactory> m_actorComponentFactory;
};
}
