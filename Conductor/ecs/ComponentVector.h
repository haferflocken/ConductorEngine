#pragma once

#include <collection/IndexIterator.h>
#include <traits/IsMemCopyAFullCopy.h>
#include <unit/CountUnits.h>
#include <util/StringHash.h>

namespace ECS
{
class Component;
class ComponentID;
class ComponentReflector;
class EntityManager;

/**
 * A vector of components of a certain type stored in contiguous memory.
 */
class ComponentVector
{
public:
	using value_type = Component;
	using const_iterator = Collection::IndexIterator<const ComponentVector, const Component>;
	using iterator = Collection::IndexIterator<ComponentVector, Component>;

	ComponentVector() = default;

	ComponentVector(const Util::StringHash componentType, const Unit::ByteCount64 alignedComponentSize,
		const uint32_t initialCapacity = 8);

	Util::StringHash GetComponentType() const { return m_componentType; }

	uint32_t Size() const { return m_count; }
	uint32_t Capacity() const { return m_capacity; }
	bool IsEmpty() const { return m_count == 0; }

	Component& operator[](const size_t i) { return reinterpret_cast<Component&>(m_data[i * m_alignedComponentSize.GetN()]); }
	const Component& operator[](const size_t i) const { return reinterpret_cast<const Component&>(m_data[i * m_alignedComponentSize.GetN()]); }

	iterator begin() { return iterator(*this, 0); }
	const_iterator begin() const { return const_iterator(*this, 0); }
	const_iterator cbegin() const { return begin(); }

	iterator end() { return iterator(*this, m_count); }
	const_iterator end() const { return const_iterator(*this, m_count); }
	const_iterator cend() const { return end(); }

	template <typename T, typename... Args>
	T& Emplace(Args&&... args);

	void Remove(const ComponentID id, const ComponentReflector& componentReflector);

private:
	template <typename T>
	void EnsureCapacity(const size_t desiredCapacity);

	Util::StringHash m_componentType{};
	Unit::ByteCount64 m_alignedComponentSize{ 0 };

	uint8_t* m_data{ nullptr };
	uint32_t m_capacity{ 0 };
	uint32_t m_count{ 0 };
};

inline ComponentVector::ComponentVector(const Util::StringHash componentType,
	const Unit::ByteCount64 alignedComponentSize, const uint32_t initialCapacity)
	: m_componentType(componentType)
	, m_alignedComponentSize(alignedComponentSize)
	, m_data(static_cast<uint8_t*>(malloc(initialCapacity * alignedComponentSize.GetN())))
	, m_capacity(initialCapacity)
{}

template <typename T, typename... Args>
inline T& ComponentVector::Emplace(Args&&... args)
{
	const size_t i = m_count;
	EnsureCapacity<T>(i + 1);

	T* const destination = static_cast<T*>(&((*this)[i]));
	new (destination) T(std::forward<Args>(args)...);
	m_count += 1;
	return *destination;
}

template <typename T>
inline void ComponentVector::EnsureCapacity(const size_t desiredCapacity)
{
	if (desiredCapacity > Capacity())
	{
		// If we need more room, double our capacity as many times as we need to.
		uint32_t newCapacity = Capacity() * 2;
		while (newCapacity < desiredCapacity)
		{
			newCapacity *= 2;
		}
		T* const newData = static_cast<T*>(malloc(newCapacity * m_alignedComponentSize.GetN()));

		// Move the contents of the old buffer into the new one.
		if (Traits::IsMemCopyAFullCopy<T>::value)
		{
			memcpy(newData, m_data, m_count * m_alignedComponentSize.GetN());
		}
		else
		{
			T* const oldData = reinterpret_cast<T*>(m_data);
			for (size_t i = 0, iEnd = m_count; i < iEnd; ++i)
			{
				new (&newData[i]) T(std::move(oldData[i]));
			}
		}

		// Move the new buffer into m_data and delete the old buffer.
		if (m_data != nullptr)
		{
			delete m_data;
		}
		m_data = reinterpret_cast<uint8_t*>(newData);
	}
}
}
