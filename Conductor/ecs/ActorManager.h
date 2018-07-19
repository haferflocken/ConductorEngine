#pragma once

#include <collection/ArrayView.h>
#include <collection/Vector.h>
#include <collection/VectorMap.h>

#include <ecs/ActorComponentGroupVector.h>
#include <ecs/ActorComponentID.h>
#include <ecs/ActorID.h>

#include <mem/UniquePtr.h>

#include <util/StringHash.h>

#include <functional>
#include <type_traits>

namespace Behave
{
class BehaveContext;
}

namespace Collection
{
template <typename T>
class ArrayView;
}

namespace ECS
{
class Actor;
class ActorComponent;
class ActorComponentFactory;
class ActorComponentGroupVector;
class ActorComponentVector;
class ActorInfo;
class BehaviourSystem;

/**
 * An actor manager owns and updates actors.
 */
class ActorManager final
{
public:
	ActorManager(const ActorComponentFactory& componentFactory);
	~ActorManager();

	Actor& CreateActor(const ActorInfo& actorInfo);

	Actor* FindActor(const ActorID id);
	const Actor* FindActor(const ActorID id) const;

	ActorComponent* FindComponent(const ActorComponentID id);
	const ActorComponent* FindComponent(const ActorComponentID id) const;

	size_t FindComponentIndex(const ActorComponentID id) const;

	Actor& GetActorByIndex(const size_t index);
	ActorComponent& GetComponentByIndex(const Util::StringHash typeHash, const size_t index);

	void RemoveComponent(const ActorComponentID id);

	template <typename BehaviourSystemType>
	void RegisterBehaviourSystem(Mem::UniquePtr<BehaviourSystemType>&& system);

	void Update(const Behave::BehaveContext& context);

private:
	struct RegisteredBehaviourSystem;
	using BehaviourSystemUpdateFn = void(*)(ActorManager&, const Behave::BehaveContext&, RegisteredBehaviourSystem&);

	struct RegisteredBehaviourSystem
	{
		RegisteredBehaviourSystem();
		RegisteredBehaviourSystem(Mem::UniquePtr<BehaviourSystem>&& system, BehaviourSystemUpdateFn updateFunction);

		Mem::UniquePtr<BehaviourSystem> m_behaviourSystem;
		BehaviourSystemUpdateFn m_updateFunction;
		ActorComponentGroupVector m_actorComponentGroups;
		Collection::Vector<std::function<void()>> m_deferredFunctions;
	};

	struct BehaviourSystemExecutionGroup
	{
		Collection::Vector<RegisteredBehaviourSystem> m_systems;
	};

	void RegisterBehaviourSystem(Mem::UniquePtr<BehaviourSystem>&& system, BehaviourSystemUpdateFn updateFn);

	void AddActorComponentsToBehaviourSystems(const Collection::ArrayView<Actor>& actorsToAdd);

	void UpdateBehaviourSystems(const Behave::BehaveContext& context);

	// The factory this manager uses to create actor components.
	const ActorComponentFactory& m_actorComponentFactory;

	// The actors this manager is in charge of updating, sorted by ID.
	Collection::Vector<Actor> m_actors{};

	// The actor components this manager owns on behalf of its actors, grouped by type.
	Collection::VectorMap<Util::StringHash, ActorComponentVector> m_actorComponents{};

	// The next actor ID that will be assigned.
	ActorID m_nextActorID{ 0 };

	// The next actor component ID that will be assigned.
	size_t m_nextActorComponentID{ 0 };

	// The behaviour systems that this actor manager is running, sorted into groups which can run concurrently.
	Collection::Vector<BehaviourSystemExecutionGroup> m_behaviourSystemExecutionGroups{};

	// Whether or not the actor component group vectors need to be recalculated.
	bool m_actorComponentGroupVectorsNeedRecalculation{ false };
};

template <typename BehaviourSystemType>
void ActorManager::RegisterBehaviourSystem(Mem::UniquePtr<BehaviourSystemType>&& system)
{
	struct SystemTypeFunctions
	{
		static void Update(ActorManager& actorManager, const BehaveContext& context,
			RegisteredBehaviourSystem& registeredSystem)
		{
			const auto& system = static_cast<const BehaviourSystemType&>(*registeredSystem.m_behaviourSystem);
			const auto actorComponentGroupsView =
				registeredSystem.m_actorComponentGroups.GetView<BehaviourSystemType::ActorComponentGroupType>();

			system.Update(actorManager, context, actorComponentGroupsView, registeredSystem.m_deferredFunctions);
		}
	};
	RegisterBehaviourSystem(std::move(system), &SystemTypeFunctions::Update);
}
}
