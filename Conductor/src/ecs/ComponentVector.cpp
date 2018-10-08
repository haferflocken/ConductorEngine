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

ECS::ComponentVector::ComponentVector(ComponentVector&& other)
	: m_componentReflector(other.m_componentReflector)
	, m_componentType(other.m_componentType)
	, m_allocator(std::move(other.m_allocator))
	, m_keyLookup(std::move(other.m_keyLookup))
{
}

ECS::ComponentVector& ECS::ComponentVector::operator=(ComponentVector&& rhs)
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
		const auto destructorFn = m_componentReflector->FindDestructorFunction(m_componentType);
		for (size_t i = 0, n = m_keyLookup.GetNumBuckets(); i < n; ++i)
		{
			for (auto&& componentPtr : m_keyLookup.GetBucketViewAt(i).m_values)
			{
				destructorFn(*componentPtr);
				m_allocator.Free(componentPtr);
			}
		}
	}
}

void ECS::ComponentVector::Copy(const ComponentVector& other)
{
	Dev::FatalAssert(m_componentType == other.m_componentType,
		"ComponentVector::Copy() does not support changing component types.");
	
	Clear();

	const auto* const transmissionFns = m_componentReflector->FindTransmissionFunctions(m_componentType);
	Dev::FatalAssert(transmissionFns != nullptr, "ComponentVector::Copy() only supports networked component types.");
	for (size_t i = 0, iEnd = other.m_keyLookup.GetNumBuckets(); i < iEnd; ++i)
	{
		const auto& bucketView = other.m_keyLookup.GetBucketViewAt(i);
		for (size_t j = 0, jEnd = bucketView.m_keys.Size(); j < jEnd; ++j)
		{
			const ComponentID componentID = bucketView.m_keys[j];
			const Component& component = *bucketView.m_values[j];
			Component* const destination = reinterpret_cast<Component*>(m_allocator.Alloc());
			transmissionFns->m_copyConstructFunction(destination, component);
			m_keyLookup[destination->m_id] = destination;
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
	const auto destructorFn = m_componentReflector->FindDestructorFunction(m_componentType);

	Component* component = nullptr;
	for (auto&& componentIDValue : ids)
	{
		const ComponentID componentID{ m_componentType, componentIDValue };
		Component* component = nullptr;
		if (!m_keyLookup.TryRemove(componentID, &component))
		{
			return;
		}

		destructorFn(*component);
		m_allocator.Free(component);
	}
}
