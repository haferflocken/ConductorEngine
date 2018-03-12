#pragma once

#include <behave/ActorComponentGroupVector.h>
#include <behave/ActorComponentID.h>
#include <behave/ActorID.h>

#include <collection/ArrayView.h>
#include <collection/Vector.h>
#include <collection/VectorMap.h>

#include <mem/UniquePtr.h>

#include <util/StringHash.h>

#include <functional>

namespace Collection
{
template <typename T>
class ArrayView;
}

namespace Behave
{
class Actor;
class ActorComponent;
class ActorComponentFactory;
class ActorComponentGroupVector;
class ActorComponentVector;
class ActorInfo;
class BehaviourSystem;
class BehaviourTree;
class BehaviourTreeContext;
class BehaviourTreeEvaluator;

/**
 * An actor manager owns and updates actors.
 */
class ActorManager final
{
public:
	using BehaviourSystemUpdateFn = void(*)(const BehaviourSystem&, ActorComponentGroupVector&);

	ActorManager(const ActorComponentFactory& componentFactory);
	~ActorManager();

	Actor& CreateActor(const ActorInfo& actorInfo);

	Actor* FindActor(const ActorID id);
	const Actor* FindActor(const ActorID id) const;

	ActorComponent* FindComponent(const ActorComponentID id);
	const ActorComponent* FindComponent(const ActorComponentID id) const;

	size_t FindComponentIndex(const ActorComponentID id) const;

	void RemoveComponent(const ActorComponentID id);

	template <typename BehaviourSystemType>
	void RegisterBehaviourSystem(Mem::UniquePtr<BehaviourSystemType>&& system);

	void Update(const BehaviourTreeContext& treeContext);

private:
	struct RegisteredBehaviourSystem
	{
		RegisteredBehaviourSystem();
		RegisteredBehaviourSystem(Mem::UniquePtr<BehaviourSystem>&& system, BehaviourSystemUpdateFn updateFunction);

		Mem::UniquePtr<BehaviourSystem> m_behaviourSystem;
		BehaviourSystemUpdateFn m_updateFunction;
		ActorComponentGroupVector m_componentGroups;
	};

	struct BehaviourSystemExecutionGroup
	{
		Collection::Vector<RegisteredBehaviourSystem> m_systems;
	};

	void RegisterBehaviourSystem(Mem::UniquePtr<BehaviourSystem>&& system, BehaviourSystemUpdateFn updateFn);

	void AddActorComponentsToBehaviourSystems(const Collection::ArrayView<Actor>& actorsToAdd);

	void UpdateBehaviourTrees(const BehaviourTreeContext& context);
	void UpdateBehaviourSystems();

	// The factory this manager uses to create actor components.
	const ActorComponentFactory& m_actorComponentFactory;

	// The actors this manager is in charge of updating, sorted by ID.
	Collection::Vector<Actor> m_actors;

	// The actor components this manager owns on behalf of its actors, grouped by type.
	Collection::VectorMap<Util::StringHash, ActorComponentVector> m_actorComponents;

	// The next actor ID that will be assigned.
	ActorID m_nextActorID;

	// The next actor component ID that will be assigned.
	size_t m_nextActorComponentID;

	// The behaviour systems that this actor manager is running, sorted into groups which can run concurrently.
	Collection::Vector<BehaviourSystemExecutionGroup> m_behaviourSystemExecutionGroups;
};

template <typename BehaviourSystemType>
void ActorManager::RegisterBehaviourSystem(Mem::UniquePtr<BehaviourSystemType>&& system)
{
	struct SystemTypeFunctions
	{
		static void Update(const BehaviourSystem& system, ActorComponentGroupVector& componentGroups)
		{
			const auto view = componentGroups.GetView<BehaviourSystemType::ComponentGroupType>();
			static_cast<const BehaviourSystemType&>(system).Update(view);
		}
	};
	RegisterBehaviourSystem(std::move(system), &SystemTypeFunctions::Update);
}
}
