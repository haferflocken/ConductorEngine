#pragma once

#include <conductor/IGameData.h>
#include <file/Path.h>
#include <mem/UniquePtr.h>

namespace Behave
{
class ActorComponentFactory;
class ActorComponentInfoFactory;
class ActorInfoManager;
class BehaviourNodeFactory;
class BehaviourTreeManager;
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
	
	const Behave::ActorComponentInfoFactory& GetActorComponentInfoFactory() const { return *m_actorComponentInfoFactory; }
	const Behave::ActorInfoManager& GetActorInfoManager() const { return *m_actorInfoManager; }
	
	const Behave::ActorComponentFactory& GetActorComponentFactory() const { return *m_actorComponentFactory; }

	void LoadBehaviourTreesInDirectory(const File::Path& directory);
	void LoadActorInfosInDirectory(const File::Path& directory);

private:
	Mem::UniquePtr<Behave::BehaviourNodeFactory> m_behaviourNodeFactory;
	Mem::UniquePtr<Behave::BehaviourTreeManager> m_behaviourTreeManager;

	Mem::UniquePtr<Behave::ActorComponentInfoFactory> m_actorComponentInfoFactory;
	Mem::UniquePtr<Behave::ActorInfoManager> m_actorInfoManager;

	Mem::UniquePtr<Behave::ActorComponentFactory> m_actorComponentFactory;
};
}
