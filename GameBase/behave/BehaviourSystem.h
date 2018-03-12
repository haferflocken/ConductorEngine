#pragma once

#include <behave/ActorComponentGroup.h>
#include <behave/ActorComponentGroupVector.h>
#include <collection/Vector.h>
#include <util/StringHash.h>
#include <util/VariadicUtil.h>

namespace Behave
{
class ActorComponentGroupVector;

/**
 * A behaviour system updates actors which have a set of components which match the system's input components.
 * Behaviour systems should be defined by extending BehaviourSystemTempl.
 * All behaviour systems must define an update function which encapsulates their logic with this signature:
 *   void Update(const Collection::ArrayView<ComponentGroupType>& componentGroups) const;
 */
class BehaviourSystem
{
public:
	explicit BehaviourSystem(Collection::Vector<Util::StringHash>&& immutableComponentTypes,
		Collection::Vector<Util::StringHash>&& mutableComponentTypes)
		: m_immutableComponentTypes(std::move(immutableComponentTypes))
		, m_mutableComponentTypes(std::move(mutableComponentTypes))
	{}

	virtual ~BehaviourSystem() {}

	const Collection::Vector<Util::StringHash>& GetImmutableComponentTypes() const { return m_immutableComponentTypes; }
	const Collection::Vector<Util::StringHash>& GetMutableComponentTypes() const { return m_mutableComponentTypes; }

private:
	Collection::Vector<Util::StringHash> m_immutableComponentTypes;
	Collection::Vector<Util::StringHash> m_mutableComponentTypes;
};

namespace Detail
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

template <typename ComponentTypeList>
using ActorComponentGroupForTypeList = decltype(ActorComponentGroupForTypeListFn(ComponentTypeList()));
}

template <typename ImmutableComponentTL, typename MutableComponentTL>
class BehaviourSystemTempl : public BehaviourSystem
{
public:
	using ImmutableComponentTypeList = typename ImmutableComponentTL::ConstList;
	using MutableComponentTypeList = MutableComponentTL;

	using ComponentTypeList = typename ImmutableComponentTypeList::template ConcatType<MutableComponentTypeList>;
	using ComponentGroupType = Detail::ActorComponentGroupForTypeList<ComponentTypeList>;

	BehaviourSystemTempl()
		: BehaviourSystem(Detail::MapTupleTypeHashesToVector<ImmutableComponentTypeList>(),
			Detail::MapTupleTypeHashesToVector<MutableComponentTypeList>())
	{}
};
}
