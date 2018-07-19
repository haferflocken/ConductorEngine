#pragma once

#include <ecs/ActorComponentGroup.h>
#include <ecs/ActorComponentGroupVector.h>

#include <collection/Vector.h>
#include <util/StringHash.h>
#include <util/VariadicUtil.h>

namespace ECS
{
class ActorComponentGroupVector;

/**
 * A behaviour system updates actors which have a set of components which match the system's input components.
 * Behaviour systems should be defined by extending BehaviourSystemTempl.
 * All behaviour systems must define an update function which encapsulates their logic with this signature:
 *   void Update(const Collection::ArrayView<ActorComponentGroupType>& componentGroups) const;
 */
class BehaviourSystem
{
public:
	explicit BehaviourSystem(Collection::Vector<Util::StringHash>&& immutableTypes,
		Collection::Vector<Util::StringHash>&& mutableTypes)
		: m_immutableTypes(std::move(immutableTypes))
		, m_mutableTypes(std::move(mutableTypes))
	{}

	virtual ~BehaviourSystem() {}

	const Collection::Vector<Util::StringHash>& GetImmutableTypes() const { return m_immutableTypes; }
	const Collection::Vector<Util::StringHash>& GetMutableTypes() const { return m_mutableTypes; }

private:
	Collection::Vector<Util::StringHash> m_immutableTypes;
	Collection::Vector<Util::StringHash> m_mutableTypes;
};

namespace BehaviourSystemDetail
{
template <typename TypeList, size_t... Indices>
Collection::Vector<Util::StringHash> MapTupleTypeHashesToVector(std::index_sequence<Indices...>)
{
	return Collection::Vector<Util::StringHash>({ TypeList::Get<Indices>::Info::sk_typeHash... });
}

template <typename TypeList>
Collection::Vector<Util::StringHash> MapTupleTypeHashesToVector()
{
	return MapTupleTypeHashesToVector<TypeList>(std::make_index_sequence<TypeList::Size>());
}

template <typename... ComponentTypes>
ActorComponentGroup<ComponentTypes...> ActorComponentGroupForTypeListFn(Util::TypeList<ComponentTypes...>)
{
	return {};
}

template <typename ActorComponentTypeList>
using ActorComponentGroupForTypeList = decltype(ActorComponentGroupForTypeListFn(ActorComponentTypeList()));
}

template <typename ImmutableTL, typename MutableTL>
class BehaviourSystemTempl : public BehaviourSystem
{
public:
	using ImmutableTypesList = typename ImmutableTL::ConstList;
	using MutableTypesList = MutableTL;

	using ActorComponentTypeList = typename ImmutableTypesList::template ConcatType<MutableTypesList>;
	using ActorComponentGroupType = BehaviourSystemDetail::ActorComponentGroupForTypeList<ActorComponentTypeList>;

	BehaviourSystemTempl()
		: BehaviourSystem(BehaviourSystemDetail::MapTupleTypeHashesToVector<ImmutableTypesList>(),
			BehaviourSystemDetail::MapTupleTypeHashesToVector<MutableTypesList>())
	{}
};
}
