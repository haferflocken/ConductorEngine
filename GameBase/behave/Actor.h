#pragma once

#include <behave/ActorComponentID.h>
#include <behave/ActorID.h>
#include <behave/BehaviourTreeEvaluator.h>

#include <Collection/Vector.h>

namespace Behave
{
class ActorWorldContext;
class BehaviourNodeState;
class BehaviourTree;

/**
 * An actor runs behaviour trees.
 */
class Actor final
{
public:
	explicit Actor(const ActorID& id)
		: m_id(id)
		, m_treeEvaluators()
		, m_components()
	{}

	Actor(const Actor&) = delete;
	Actor& operator=(const Actor&) = delete;

	Actor(Actor&&) = default;
	Actor& operator=(Actor&&) = default;

	~Actor() {}

	template <typename ComponentType>
	ActorComponentID FindComponentID() const;

public:
	// A unique ID for this actor.
	ActorID m_id;
	// TODO(refactor) behaviour tree component The evaluation contexts for the behaviour trees this actor is running.
	Collection::Vector<BehaviourTreeEvaluator> m_treeEvaluators;
	// The components this actor is composed of.
	Collection::Vector<ActorComponentID> m_components;
};

template <typename ComponentType>
ActorComponentID Actor::FindComponentID() const
{
	// TODO This is currently a linear search, which callers will typically follow up with a binary search
	//      to actually get the component. It's worth remembering that this may be a performance concern.
	for (const auto& id : m_components)
	{
		if (id.GetType() == ComponentType::Info::sk_typeHash)
		{
			return id;
		}
	}
	return ActorComponentID();
}
}
