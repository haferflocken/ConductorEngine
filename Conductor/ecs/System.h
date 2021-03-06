#pragma once

#include <ecs/ECSGroup.h>
#include <ecs/ECSGroupVector.h>

#include <collection/Vector.h>
#include <unit/Time.h>
#include <util/StringHash.h>
#include <util/VariadicUtil.h>

namespace ECS
{
class EntityManager;

/**
 * A System updates entities which have a set of components which match the system's input components.
 * Systems should be defined by extending SystemTempl.
 * All systems must define an update function which encapsulates their logic with this signature:
 *   void Update(const Unit::Time::Millisecond delta,
 *      const Collection::ArrayView<ECSGroupType>& ecsGroups,
 *      Collection::Vector<std::function<void(EntityManager&)>>& deferredFunctions);
 * 
 * Systems that have SystemBindingType::Extended must define all of the following:
 *   void NotifyOfEntityAdded(const EntityID id, const ECSGroupType& group);
 *   void NotifyOfEntityRemoved(const EntityID id, const ECSGroupType& group);
 *
 * Systems may, but are not required to, override the virtual functions of ECS::System.
 */
class System
{
public:
	System(Collection::Vector<ECS::ComponentType>&& immutableTypes,
		Collection::Vector<ECS::ComponentType>&& mutableTypes)
		: m_immutableTypes(std::move(immutableTypes))
		, m_mutableTypes(std::move(mutableTypes))
	{}

	virtual ~System() {}

	virtual void NotifyOfShutdown(ECS::EntityManager&) {}

	const Collection::Vector<ECS::ComponentType>& GetImmutableTypes() const { return m_immutableTypes; }
	const Collection::Vector<ECS::ComponentType>& GetMutableTypes() const { return m_mutableTypes; }

private:
	Collection::Vector<ECS::ComponentType> m_immutableTypes;
	Collection::Vector<ECS::ComponentType> m_mutableTypes;
};

/**
 * All systems define a binding type to indicate what functions they need bound.
 */
enum class SystemBindingType
{
	Standard,
	Extended
};

namespace SystemDetail
{
template <typename TypeList, size_t... Indices>
Collection::Vector<ECS::ComponentType> MapTupleTypesToVector(std::index_sequence<Indices...>)
{
	return Collection::Vector<ECS::ComponentType>({ TypeList::Get<Indices>::k_type... });
}

template <typename TypeList>
Collection::Vector<ECS::ComponentType> MapTupleTypesToVector()
{
	return MapTupleTypesToVector<TypeList>(std::make_index_sequence<TypeList::Size>());
}

template <typename... ComponentTypes>
ECSGroup<ComponentTypes...> ECSGroupForTypeListFn(Util::TypeList<ComponentTypes...>)
{
	return {};
}

template <typename ECSTypeList>
using ECSGroupForTypeList = decltype(ECSGroupForTypeListFn(ECSTypeList()));
}

template <typename ImmutableTL, typename MutableTL, SystemBindingType BindingType = SystemBindingType::Standard>
class SystemTempl : public System
{
	using ThisType = SystemTempl<ImmutableTL, MutableTL>;

public:
	using ImmutableTypesList = typename ImmutableTL::ConstList;
	using MutableTypesList = MutableTL;
	static constexpr SystemBindingType k_bindingType = BindingType;

	using ECSTypeList = typename ImmutableTypesList::template ConcatType<MutableTypesList>;
	using ECSGroupType = SystemDetail::ECSGroupForTypeList<ECSTypeList>;

	template <typename OtherSystemType>
	static constexpr bool WritesToInputsOf() { return OtherSystemType::ImmutableTypesList::template ContainsType<const ::ECS::Entity>() || MutableTypesList::ConstList::ContainsAny(OtherSystemType::ImmutableTypesList()); }

	template <typename... OtherSystemTypes>
	static constexpr bool WritesToInputsOfAny() { return (... || WritesToInputsOf<OtherSystemTypes>()); }

	template <typename OtherSystemType>
	static constexpr bool WritesToOutputsOf() { return MutableTypesList::template ContainsType<::ECS::Entity>() || MutableTypesList::ContainsAny(OtherSystemType::MutableTypesList()); }

	template <typename... OtherSystemTypes>
	static constexpr bool WritesToOutputsOfAny() { return (... || WritesToOutputsOf<OtherSystemTypes>()); }

	template <typename... OtherSystemTypes>
	static constexpr bool IsWriteCompatibleWithAll() { return (!WritesToInputsOfAny<OtherSystemTypes...>()) && (!WritesToOutputsOfAny<OtherSystemTypes...>()); }

	template <typename... OtherSystemTypes>
	static constexpr bool IsWriteCompatibleWithAll(Util::TypeList<OtherSystemTypes...>) { return IsWriteCompatibleWithAll<OtherSystemTypes...>(); }

	SystemTempl()
		: System(SystemDetail::MapTupleTypesToVector<ImmutableTypesList>(),
			SystemDetail::MapTupleTypesToVector<MutableTypesList>())
	{}
};
}
