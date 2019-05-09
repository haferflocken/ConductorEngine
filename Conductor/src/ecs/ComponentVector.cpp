#include <ecs/ComponentVector.h>

#include <ecs/Component.h>
#include <ecs/ComponentID.h>
#include <ecs/ComponentReflector.h>

#include <random>

ECS::ComponentVector::ComponentVector()
	: m_keyLookup(ComponentIDHashFunctor(), 6)
{}

ECS::ComponentVector::~ComponentVector()
{
	Clear();
}

ECS::ComponentVector::ComponentVector(
	const ComponentReflector& componentReflector,
	const ComponentType componentType,
	const Unit::ByteCount64 componentSize,
	const Unit::ByteCount64 componentAlignment)
	: m_componentReflector(&componentReflector)
	, m_componentType(componentType)
	, m_allocator(componentAlignment.GetN(), componentSize.GetN())
	, m_keyLookup(ComponentIDHashFunctor(), 6)
{
}

ECS::ComponentVector::ComponentVector(ComponentVector&& other) noexcept
	: m_componentReflector(other.m_componentReflector)
	, m_componentType(other.m_componentType)
	, m_allocator(std::move(other.m_allocator))
	, m_keyLookup(std::move(other.m_keyLookup))
{
}

ECS::ComponentVector& ECS::ComponentVector::operator=(ComponentVector&& rhs) noexcept
{
	Clear();

	m_componentReflector = rhs.m_componentReflector;
	m_componentType = rhs.m_componentType;
	m_allocator = std::move(rhs.m_allocator);
	m_keyLookup = std::move(rhs.m_keyLookup);
	
	return *this;
}

void ECS::ComponentVector::Clear()
{
	if (m_componentReflector != nullptr)
	{
		const auto& componentFunctions = m_componentReflector->FindComponentFunctions(m_componentType);
		for (size_t i = 0, n = m_keyLookup.GetNumBuckets(); i < n; ++i)
		{
			for (auto&& componentPtr : m_keyLookup.GetBucketViewAt(i).m_values)
			{
				componentFunctions.m_destructorFunction(*componentPtr);
				m_allocator.Free(componentPtr);
			}
		}
	}
}

ECS::Component* ECS::ComponentVector::Find(const ComponentID& key)
{
	const auto iter = m_keyLookup.Find(key);
	return (iter != nullptr) ? *iter : nullptr;
}

const ECS::Component* ECS::ComponentVector::Find(const ComponentID& key) const
{
	const auto iter = m_keyLookup.Find(key);
	return (iter != nullptr) ? *iter : nullptr;
}

void ECS::ComponentVector::Remove(const ComponentID id)
{
	Component* component = nullptr;
	if (!m_keyLookup.TryRemove(id, &component))
	{
		return;
	}

	m_componentReflector->DestroyComponent(*component);
	m_allocator.Free(component);
}

void ECS::ComponentVector::RemoveSorted(const Collection::ArrayView<const uint64_t> ids)
{
	const auto componentFunctions = m_componentReflector->FindComponentFunctions(m_componentType);

	Component* component = nullptr;
	for (auto&& componentIDValue : ids)
	{
		const ComponentID componentID{ m_componentType, componentIDValue };
		Component* component = nullptr;
		if (!m_keyLookup.TryRemove(componentID, &component))
		{
			return;
		}

		componentFunctions.m_destructorFunction(*component);
		m_allocator.Free(component);
	}
}
