#pragma once

#include <ecs/ComponentID.h>

#include <collection/ArrayView.h>
#include <collection/HashMap.h>
#include <collection/LinearBlockAllocator.h>
#include <traits/IsMemCopyAFullCopy.h>
#include <unit/CountUnits.h>

namespace ECS
{
class Component;
class ComponentReflector;
class EntityManager;

/**
 * A collection of components of a certain type stored in mostly contiguous memory.
 */
class ComponentVector
{
public:
	using value_type = Component;
	using iterator = Collection::LinearBlockAllocator::iterator;
	using const_iterator = Collection::LinearBlockAllocator::const_iterator;

	ComponentVector();
	~ComponentVector();

	ComponentVector(const ComponentReflector& componentReflector, const ComponentType componentType,
		const Unit::ByteCount64 componentSize, const Unit::ByteCount64 componentAlignment);

	ComponentVector(const ComponentVector&) = delete;
	ComponentVector& operator=(const ComponentVector&) = delete;

	ComponentVector(ComponentVector&& other) noexcept;
	ComponentVector& operator=(ComponentVector&& rhs) noexcept;

	void Clear();

	ComponentType GetComponentType() const { return m_componentType; }

	Component* Find(const ComponentID& key);
	const Component* Find(const ComponentID& key) const;

	template <typename T, typename... Args>
	T& Emplace(Args&&... args);

	void Remove(const ComponentID id);
	void RemoveSorted(const Collection::ArrayView<const uint64_t> ids);

	// These allocate over the internal allocator and as such the values iterated over
	// must be reinterpret_cast to Component&.
	iterator begin() { return m_allocator.begin(); }
	const_iterator begin() const { return m_allocator.begin(); }
	const_iterator cbegin() const { return m_allocator.cbegin(); }

	iterator end() { return m_allocator.end(); }
	const_iterator end() const { return m_allocator.end(); }
	const_iterator cend() const { return m_allocator.cend(); }

private:
	class ComponentIDHashFunctor final : public Collection::I64HashFunctor
	{
	public:
		uint64_t Hash(const ComponentID& key) const { return I64HashFunctor::Hash(key.GetUniqueID()); }
	};

	const ComponentReflector* m_componentReflector{ nullptr };

	ComponentType m_componentType{};
	Collection::LinearBlockAllocator m_allocator{};
	Collection::HashMap<ComponentID, Component*, ComponentIDHashFunctor> m_keyLookup;
};

template <typename T, typename... Args>
inline T& ComponentVector::Emplace(Args&&... args)
{
	T* const destination = reinterpret_cast<T*>(m_allocator.Alloc());
	new (destination) T(std::forward<Args>(args)...);
	m_keyLookup[destination->m_id] = destination;
	return *destination;
}
}
