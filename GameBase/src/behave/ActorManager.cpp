#include <behave/ActorManager.h>

#include <behave/Actor.h>
#include <behave/ActorComponent.h>
#include <behave/ActorComponentFactory.h>
#include <behave/ActorComponentGroupVector.h>
#include <behave/ActorComponentVector.h>
#include <behave/ActorInfo.h>
#include <behave/BehaveContext.h>
#include <behave/BehaviourNode.h>
#include <behave/BehaviourNodeState.h>
#include <behave/BehaviourSystem.h>
#include <behave/BehaviourTree.h>
#include <behave/systems/BehaviourTreeEvaluationSystem.h>

#include <collection/ArrayView.h>

#include <algorithm>
#include <set>
#include <vector>

Behave::ActorManager::ActorManager(const ActorComponentFactory& componentFactory)
	: m_actorComponentFactory(componentFactory)
	, m_actors()
	, m_actorComponents()
	, m_nextActorID(0)
	, m_nextActorComponentID(0)
{
	RegisterBehaviourSystem(Mem::MakeUnique<Systems::BehaviourTreeEvaluationSystem>());
}

Behave::ActorManager::~ActorManager()
{
}

Behave::Actor& Behave::ActorManager::CreateActor(const ActorInfo& actorInfo)
{
	// Create the actor.
	const ActorID actorID = m_nextActorID;
	Actor& actor = m_actors.Emplace(actorID);

	// Create the actor's components using our component factory.
	for (const auto& componentInfo : actorInfo.m_componentInfos)
	{
		const Util::StringHash componentTypeHash = componentInfo->GetTypeHash();
		const ActorComponentID componentID{ componentTypeHash, m_nextActorComponentID };

		ActorComponentVector& componentVector = m_actorComponents[componentTypeHash];
		if (componentVector.GetComponentType() == Util::StringHash())
		{
			// This is a type that has not yet been encountered and therefore must be initialized.
			componentVector = ActorComponentVector(componentTypeHash,
				m_actorComponentFactory.GetSizeOfComponentInBytes(componentTypeHash));
		}
		Dev::FatalAssert(componentVector.GetComponentType() == componentTypeHash,
			"Mismatch between component vector type and the key it is stored at.");

		if (!m_actorComponentFactory.TryMakeComponent(*componentInfo, componentID, componentVector))
		{
			Dev::LogWarning("Failed to create component of type [%s].", componentInfo->GetTypeName());
			continue;
		}

		++m_nextActorComponentID;
		actor.m_components.Add(componentID);
	}
	
	// Update the next unique ID.
	m_nextActorID = ActorID(actorID.GetUniqueID() + 1);

	// Add the actor's components to the behaviour system execution groups.
	AddActorComponentsToBehaviourSystems(Collection::ArrayView<Actor>(&actor, 1));

	// Return the actor after it is fully initialized.
	return actor;
}

Behave::Actor* Behave::ActorManager::FindActor(const ActorID id)
{
	// Implemented using the const variant.
	return const_cast<Actor*>(static_cast<const ActorManager*>(this)->FindActor(id));
}

const Behave::Actor* Behave::ActorManager::FindActor(const ActorID id) const
{
	const auto itr = std::lower_bound(m_actors.begin(), m_actors.end(), id,
		[](const Actor& actor, const ActorID& id)
	{
		return actor.m_id < id;
	});
	if (itr == m_actors.end() || itr->m_id != id)
	{
		return nullptr;
	}
	return &*itr;
}

Behave::ActorComponent* Behave::ActorManager::FindComponent(const ActorComponentID id)
{
	// Implemented using the const variant.
	return const_cast<ActorComponent*>(static_cast<const ActorManager*>(this)->FindComponent(id));
}

const Behave::ActorComponent* Behave::ActorManager::FindComponent(const ActorComponentID id) const
{
	const Collection::Pair<const Util::StringHash, ActorComponentVector>* const componentsEntry =
		m_actorComponents.Find(id.GetType());
	if (componentsEntry == nullptr)
	{
		return nullptr;
	}
	const ActorComponentVector& components = componentsEntry->second;

	const auto itr = std::lower_bound(components.begin(), components.end(), id,
		[](const ActorComponent& component, const ActorComponentID& id)
	{
		return component.m_id < id;
	});
	if (itr == components.end() || itr->m_id != id)
	{
		return nullptr;
	}
	return &*itr;
}

size_t Behave::ActorManager::FindComponentIndex(const ActorComponentID id) const
{
	const Collection::Pair<const Util::StringHash, ActorComponentVector>* const componentsEntry =
		m_actorComponents.Find(id.GetType());
	if (componentsEntry == nullptr)
	{
		return static_cast<size_t>(-1i64);
	}
	const ActorComponentVector& components = componentsEntry->second;

	const auto itr = std::lower_bound(components.begin(), components.end(), id,
		[](const ActorComponent& component, const ActorComponentID& id)
	{
		return component.m_id < id;
	});
	if (itr == components.end() || itr->m_id != id)
	{
		return static_cast<size_t>(-1i64);
	}
	return static_cast<size_t>(itr.GetIndex());
}

Behave::Actor& Behave::ActorManager::GetActorByIndex(const size_t index)
{
	return m_actors[index];
}

Behave::ActorComponent& Behave::ActorManager::GetComponentByIndex(const Util::StringHash typeHash, const size_t index)
{
	return m_actorComponents.Find(typeHash)->second[index];
}

void Behave::ActorManager::RemoveComponent(const ActorComponentID id)
{
	// TODO remove components from execution groups
	Collection::Pair<const Util::StringHash, ActorComponentVector>* const componentsEntry =
		m_actorComponents.Find(id.GetType());
	if (componentsEntry == nullptr)
	{
		return;
	}
	ActorComponentVector& components = componentsEntry->second;
	components.Remove(id, m_actorComponentFactory);
}

Behave::ActorManager::RegisteredBehaviourSystem::RegisteredBehaviourSystem()
	: m_behaviourSystem()
	, m_updateFunction(nullptr)
	, m_componentGroups()
{}

Behave::ActorManager::RegisteredBehaviourSystem::RegisteredBehaviourSystem(
	Mem::UniquePtr<BehaviourSystem>&& system,
	BehaviourSystemUpdateFn updateFunction)
	: m_behaviourSystem(std::move(system))
	, m_updateFunction(updateFunction)
	, m_componentGroups(
		m_behaviourSystem->GetImmutableTypes().Size() + m_behaviourSystem->GetMutableTypes().Size())
{}

void Behave::ActorManager::RegisterBehaviourSystem(
	Mem::UniquePtr<BehaviourSystem>&& system,
	BehaviourSystemUpdateFn updateFn)
{
	Dev::FatalAssert(m_actors.IsEmpty(), "Behaviour systems must be registered before actors are added to the "
		"ActorManager because there is not currently support for initializing the system's component groups.");

	// Try to find an execution group for which this system does not mutate instances of the referenced types of any system
	// in the group and for which none of the systems in the group mutate instances of the referenced types of this system.
	for (auto& executionGroup : m_behaviourSystemExecutionGroups)
	{
		std::set<Util::StringHash> groupTotalTypes;
		std::set<Util::StringHash> groupMutableTypes;
		for (const auto& registeredSystem : executionGroup.m_systems)
		{
			for (const auto& type : registeredSystem.m_behaviourSystem->GetImmutableTypes())
			{
				groupTotalTypes.insert(type);
			}
			for (const auto& type : registeredSystem.m_behaviourSystem->GetMutableTypes())
			{
				groupTotalTypes.insert(type);
				groupMutableTypes.insert(type);
			}
		}

		const bool systemMutatesGroup = std::any_of(system->GetMutableTypes().begin(), system->GetMutableTypes().end(),
			[&](const Util::StringHash& type)
		{
			return (groupTotalTypes.find(type) != groupTotalTypes.end());
		});
		if (systemMutatesGroup)
		{
			continue;
		}

		const bool groupMutatesSystem = std::any_of(groupMutableTypes.begin(), groupMutableTypes.end(),
			[&](const Util::StringHash& type)
		{
			return (std::find(system->GetImmutableTypes().begin(), system->GetImmutableTypes().end(), type)
				!= system->GetImmutableTypes().end())
				|| (std::find(system->GetMutableTypes().begin(), system->GetMutableTypes().end(), type)
					!= system->GetMutableTypes().end());
		});
		if (groupMutatesSystem)
		{
			continue;
		}

		// The system does not conflict with any other systems in the group, so add it and return.
		executionGroup.m_systems.Emplace(std::move(system), updateFn);
		return;
	}

	// If we fail to find an execution group for the system, create a new one for it.
	BehaviourSystemExecutionGroup& newGroup = m_behaviourSystemExecutionGroups.Emplace();
	newGroup.m_systems.Emplace(std::move(system), updateFn);
}

void Behave::ActorManager::AddActorComponentsToBehaviourSystems(const Collection::ArrayView<Actor>& actorsToAdd)
{
	for (auto& executionGroup : m_behaviourSystemExecutionGroups)
	{
		// Gather the actor indices and component indices each member of the execution group needs.
		for (auto& registeredSystem : executionGroup.m_systems)
		{
			const Collection::Vector<Util::StringHash>& immutableTypes =
				registeredSystem.m_behaviourSystem->GetImmutableTypes();
			const Collection::Vector<Util::StringHash>& mutableTypes =
				registeredSystem.m_behaviourSystem->GetMutableTypes();

			for (const auto& actor : actorsToAdd)
			{
				const auto TryGatherIndices = [this](const Collection::Vector<Util::StringHash>& componentTypes,
					const Actor& actor, Collection::Vector<size_t>& componentIDs)
				{
					bool foundAll = true;
					for (const auto& typeHash : componentTypes)
					{
						if (typeHash == ActorInfo::sk_typeHash)
						{
							componentIDs.Add((&actor) - m_actors.begin());
							continue;
						}

						const ActorComponentID* const idPtr = actor.m_components.Find(
							[&](const ActorComponentID& componentID) { return componentID.GetType() == typeHash; });
						if (idPtr == nullptr)
						{
							foundAll = false;
							break;
						}
						componentIDs.Add(FindComponentIndex(*idPtr));
					}
					return foundAll;
				};

				Collection::Vector<size_t> indices;
				if (!TryGatherIndices(immutableTypes, actor, indices))
				{
					continue;
				}
				if (!TryGatherIndices(mutableTypes, actor, indices))
				{
					continue;
				}

				registeredSystem.m_componentGroups.Add(indices);
			}

			// Sort the group vector in order to make the memory accesses as fast as possible.
			registeredSystem.m_componentGroups.Sort();
		}
	}
}

void Behave::ActorManager::Update(const BehaveContext& context)
{
	UpdateBehaviourSystems(context);
}

void Behave::ActorManager::UpdateBehaviourSystems(const BehaveContext& context)
{
	// Update the behaviour system execution groups. The systems in each group can update in parallel.
	for (auto& executionGroup : m_behaviourSystemExecutionGroups)
	{
		// TODO Make this parallel, since there are no read/write conflicts
		for (auto& registeredSystem : executionGroup.m_systems)
		{
			const BehaviourSystem& system = *registeredSystem.m_behaviourSystem;
			registeredSystem.m_updateFunction(system, *this, context, registeredSystem.m_componentGroups);
		}
	}
}
