#pragma once

#include <ecs/ECSGroup.h>
#include <ecs/ECSGroupVector.h>

#include <collection/Vector.h>
#include <util/StringHash.h>
#include <util/VariadicUtil.h>

namespace ECS
{
/**
 * A System updates entities which have a set of components which match the system's input components.
 * Behaviour systems should be defined by extending SystemTempl.
 * All systems must define an update function which encapsulates their logic with this signature:
 *   void Update(EntityManager& entityManager,
 *      const Collection::ArrayView<ECSGroupType>& ecsGroups,
 *      Collection::Vector<std::function<void()>>& deferredFunctions) const;
 */
class System
{
public:
	explicit System(Collection::Vector<Util::StringHash>&& immutableTypes,
		Collection::Vector<Util::StringHash>&& mutableTypes)
		: m_immutableTypes(std::move(immutableTypes))
		, m_mutableTypes(std::move(mutableTypes))
	{}

	virtual ~System() {}

	const Collection::Vector<Util::StringHash>& GetImmutableTypes() const { return m_immutableTypes; }
	const Collection::Vector<Util::StringHash>& GetMutableTypes() const { return m_mutableTypes; }

private:
	Collection::Vector<Util::StringHash> m_immutableTypes;
	Collection::Vector<Util::StringHash> m_mutableTypes;
};

namespace SystemDetail
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
ECSGroup<ComponentTypes...> ECSGroupForTypeListFn(Util::TypeList<ComponentTypes...>)
{
	return {};
}

template <typename ECSTypeList>
using ECSGroupForTypeList = decltype(ECSGroupForTypeListFn(ECSTypeList()));
}

template <typename ImmutableTL, typename MutableTL>
class SystemTempl : public System
{
public:
	using ImmutableTypesList = typename ImmutableTL::ConstList;
	using MutableTypesList = MutableTL;

	using ECSTypeList = typename ImmutableTypesList::template ConcatType<MutableTypesList>;
	using ECSGroupType = SystemDetail::ECSGroupForTypeList<ECSTypeList>;

	SystemTempl()
		: System(SystemDetail::MapTupleTypeHashesToVector<ImmutableTypesList>(),
			SystemDetail::MapTupleTypeHashesToVector<MutableTypesList>())
	{}
};
}
