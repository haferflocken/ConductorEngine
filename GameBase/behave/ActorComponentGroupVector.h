#pragma once

#include <collection/ArrayView.h>
#include <collection/Vector.h>
#include <unit/CountUnits.h>
#include <util/VariadicUtil.h>

namespace Behave
{
/**
 * Holds groups of actor indices and actor component indices in contiguous storage.
 */
class ActorComponentGroupVector
{
public:
	ActorComponentGroupVector() = default;

	explicit ActorComponentGroupVector(const uint32_t groupSize, const uint32_t initialCapacity = 8)
		: m_groupSize(groupSize)
		, m_data(groupSize * initialCapacity)
	{}

	uint32_t Size() const { return m_data.Size() / m_groupSize; }
	uint32_t Capacity() const { return m_data.Capacity() / m_groupSize; }
	uint32_t NumComponentsInGroup() const { return m_groupSize; }
	bool IsEmpty() const { return m_data.IsEmpty(); }

	// Add an actor component group to this component group vector.
	void Add(const Collection::Vector<size_t>& indices); 
	
	// Sort the component groups of this vector by the first index in each component group.
	void Sort();

	void Clear() { m_data.Clear(); }

	// Return an iterable view into this which iterates with the given group type.
	template <typename ActorComponentGroupType>
	Collection::ArrayView<ActorComponentGroupType> GetView()
	{
		Dev::FatalAssert((ActorComponentGroupType::k_size * sizeof(size_t)) == sizeof(ActorComponentGroupType),
			"Component group type has mismatch between its size constant and its actual size.");
		Dev::FatalAssert(m_groupSize == ActorComponentGroupType::k_size,
			"Component group type has the wrong number of components.");
		
		ActorComponentGroupType* const data = reinterpret_cast<ActorComponentGroupType*>(&m_data[0]);
		return Collection::ArrayView<ActorComponentGroupType>(data, Size());
	}

private:
	uint32_t m_groupSize{ 0 };
	Collection::Vector<size_t> m_data;
};
}
