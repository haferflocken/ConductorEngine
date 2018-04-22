#pragma once

#include <collection/IndexIterator.h>
#include <traits/IsMemCopyAFullCopy.h>
#include <unit/CountUnits.h>
#include <util/StringHash.h>

namespace Behave
{
class ActorComponent;
class ActorComponentFactory;
class ActorComponentID;
class ActorManager;

/**
 * A vector of actor components of a certain type stored in contiguous memory.
 */
class ActorComponentVector
{
public:
	using value_type = ActorComponent;
	using const_iterator = Collection::IndexIterator<const ActorComponentVector, const ActorComponent>;
	using iterator = Collection::IndexIterator<ActorComponentVector, ActorComponent>;

	ActorComponentVector() = default;

	explicit ActorComponentVector(const Util::StringHash componentType, const Unit::ByteCount64 alignedComponentSize,
		const uint32_t initialCapacity = 8);

	Util::StringHash GetComponentType() const { return m_componentType; }

	uint32_t Size() const { return m_count; }
	uint32_t Capacity() const { return m_capacity; }
	bool IsEmpty() const { return m_count == 0; }

	ActorComponent& operator[](const size_t i) { return reinterpret_cast<ActorComponent&>(m_data[i * m_alignedComponentSize.GetN()]); }
	const ActorComponent& operator[](const size_t i) const { return reinterpret_cast<const ActorComponent&>(m_data[i * m_alignedComponentSize.GetN()]); }

	iterator begin() { return iterator(*this, 0); }
	const_iterator begin() const { return const_iterator(*this, 0); }
	const_iterator cbegin() const { return begin(); }

	iterator end() { return iterator(*this, m_count); }
	const_iterator end() const { return const_iterator(*this, m_count); }
	const_iterator cend() const { return end(); }

	template <typename T, typename... Args>
	T& Emplace(Args&&... args);

	void Remove(const ActorComponentID id, const ActorComponentFactory& componentFactory);

private:
	template <typename T>
	void EnsureCapacity(const size_t desiredCapacity);

	Util::StringHash m_componentType{};
	Unit::ByteCount64 m_alignedComponentSize{ 0 };

	uint8_t* m_data{ nullptr };
	uint32_t m_capacity{ 0 };
	uint32_t m_count{ 0 };
};

inline ActorComponentVector::ActorComponentVector(const Util::StringHash componentType,
	const Unit::ByteCount64 alignedComponentSize, const uint32_t initialCapacity)
	: m_componentType(componentType)
	, m_alignedComponentSize(alignedComponentSize)
	, m_data(static_cast<uint8_t*>(malloc(initialCapacity * alignedComponentSize.GetN())))
	, m_capacity(initialCapacity)
{}

template <typename T, typename... Args>
inline T& ActorComponentVector::Emplace(Args&&... args)
{
	const size_t i = m_count;
	EnsureCapacity<T>(i + 1);

	T* const destination = static_cast<T*>(&((*this)[i]));
	new (destination) T(std::forward<Args>(args)...);
	m_count += 1;
	return *destination;
}

template <typename T>
inline void ActorComponentVector::EnsureCapacity(const size_t desiredCapacity)
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
