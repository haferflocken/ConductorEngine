#include <ecs/Component.h>
#include <ecs/ComponentID.h>
#include <ecs/ComponentReflector.h>
#include <ecs/ComponentVector.h>

#include <algorithm>

ECS::ComponentVector::ComponentVector() = default;

ECS::ComponentVector::~ComponentVector()
{
	if (m_data != nullptr)
	{
		const auto destructorFn = m_componentReflector->FindDestructorFunction(m_componentType);
		for (auto& component : *this)
		{
			destructorFn(component);
		}
		free(m_data);
		m_data = nullptr;
	}
}

ECS::ComponentVector::ComponentVector(
	const ComponentReflector& componentReflector,
	const Util::StringHash componentType,
	const Unit::ByteCount64 alignedComponentSize,
	const uint32_t initialCapacity)
	: m_componentReflector(&componentReflector)
	, m_componentType(componentType)
	, m_alignedComponentSize(alignedComponentSize)
	, m_data(static_cast<uint8_t*>(malloc(initialCapacity * alignedComponentSize.GetN())))
	, m_capacity(initialCapacity)
{}

ECS::ComponentVector::ComponentVector(ComponentVector&& other)
	: m_componentReflector(other.m_componentReflector)
	, m_componentType(other.m_componentType)
	, m_alignedComponentSize(other.m_alignedComponentSize)
	, m_data(other.m_data)
	, m_capacity(other.m_capacity)
	, m_count(other.m_count)
{
	other.m_data = nullptr;
	other.m_capacity = 0;
	other.m_count = 0;
}

ECS::ComponentVector& ECS::ComponentVector::operator=(ComponentVector&& rhs)
{
	if (m_data != nullptr)
	{
		const auto destructorFn = m_componentReflector->FindDestructorFunction(m_componentType);
		for (auto& component : *this)
		{
			destructorFn(component);
		}
		free(m_data);
	}

	m_componentReflector = rhs.m_componentReflector;
	m_componentType = rhs.m_componentType;
	m_alignedComponentSize = rhs.m_alignedComponentSize;
	m_data = rhs.m_data;
	m_capacity = rhs.m_capacity;
	m_count = rhs.m_count;

	rhs.m_data = nullptr;
	rhs.m_capacity = 0;
	rhs.m_count = 0;

	return *this;
}

void ECS::ComponentVector::Copy(const ComponentVector& other)
{
	Dev::FatalAssert(m_componentType == other.m_componentType
		&& m_alignedComponentSize == other.m_alignedComponentSize,
		"ComponentVector::Copy() does not support changing component types.");
	
	const auto destructorFn = m_componentReflector->FindDestructorFunction(m_componentType);
	for (auto& component : *this)
	{
		destructorFn(component);
	}
	m_count = 0;

	if (m_capacity < other.m_capacity)
	{
		free(m_data);
		m_data = static_cast<uint8_t*>(malloc(other.m_capacity * m_alignedComponentSize.GetN()));
		m_capacity = other.m_capacity;
	}

	const auto* const transmissionFns = m_componentReflector->FindTransmissionFunctions(m_componentType);
	Dev::FatalAssert(transmissionFns != nullptr, "ComponentVector::Copy() only supports networked component types.");
	for (const auto& component : other)
	{
		void* place = &(*this)[m_count];
		transmissionFns->m_copyConstructFunction(place, component);
		++m_count;
	}
}

void ECS::ComponentVector::Remove(const ComponentID id)
{
	const auto itr = std::lower_bound(begin(), end(), id,
		[](const Component& component, const ComponentID& id)
	{
		return component.m_id < id;
	});
	if (itr == end() || itr->m_id != id)
	{
		return;
	}

	Component& last = (*this)[m_count - 1];
	m_componentReflector->SwapComponents(*itr, last);

	m_componentReflector->DestroyComponent(last);
	m_count -= 1;
}

void ECS::ComponentVector::RemoveSorted(const Collection::ArrayView<const uint64_t> ids)
{
	auto idIter = ids.begin();
	const auto idsEnd = ids.end();

	for (size_t i = 0; i < m_count && idIter != idsEnd;)
	{
		Component& current = (*this)[i];

		while (*idIter < (*this)[i].m_id.GetUniqueID() && idIter != idsEnd)
		{
			++idIter;
		}
		if (*idIter == (*this)[i].m_id.GetUniqueID())
		{
			Component& last = (*this)[m_count - 1];
			m_componentReflector->SwapComponents(current, last);

			m_componentReflector->DestroyComponent(last);
			m_count -= 1;

			++idIter;
		}
		else
		{
			++i;
		}
	}
}
