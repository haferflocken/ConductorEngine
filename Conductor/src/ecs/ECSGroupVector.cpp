#include <ecs/ECSGroupVector.h>

#include <dev/Dev.h>

#include <array>

void ECS::ECSGroupVector::Add(const Collection::Vector<void*>& pointers)
{
	AMP_FATAL_ASSERT(pointers.Size() == m_groupSize,
		"Can only add ECS groups with the correct group size.");
	
	for (const auto& ptr : pointers)
	{
		m_data.Add(ptr);
	}
}

void ECS::ECSGroupVector::Remove(const Collection::Vector<void*>& pointers)
{
	AMP_FATAL_ASSERT(pointers.Size() == m_groupSize,
		"Can only remove ECS groups with the correct group size.");

	for (auto iter = m_data.begin(), iterEnd = m_data.end(); iter < iterEnd; iter += m_groupSize)
	{
		bool isMatch = true;
		for (size_t i = 0, iEnd = m_groupSize; i < iEnd; ++i)
		{
			if (iter[i] != pointers[i])
			{
				isMatch = false;
				break;
			}
		}

		if (isMatch)
		{
			const std::ptrdiff_t groupIndex = std::distance(m_data.begin(), iter);
			m_data.Remove(groupIndex, groupIndex + m_groupSize);
			return;
		}
	}

	AMP_FATAL_ERROR("Failed to remove the given group.");
}

namespace Internal_ECSGroupVector
{
template <uint32_t GroupSize>
void SortGroups(Collection::Vector<void*>& data, const uint32_t numGroups)
{
	using ArrayType = std::array<size_t, GroupSize>;

	Collection::ArrayView<ArrayType> view{ reinterpret_cast<ArrayType*>(&data[0]), numGroups };

	std::sort(view.begin(), view.end(), [](const ArrayType& lhs, const ArrayType& rhs)
	{
		return lhs[0] < rhs[0];
	});
}
}

void ECS::ECSGroupVector::Sort()
{
	using namespace Internal_ECSGroupVector;

	if (IsEmpty())
	{
		return;
	}

	switch (m_groupSize)
	{
	case 0: SortGroups<0>(m_data, Size()); break;
	case 1: SortGroups<1>(m_data, Size()); break;
	case 2: SortGroups<2>(m_data, Size()); break;
	case 3: SortGroups<3>(m_data, Size()); break;
	case 4: SortGroups<4>(m_data, Size()); break;
	case 5: SortGroups<5>(m_data, Size()); break;
	default: AMP_FATAL_ERROR("Encountered an ECS group with more elements than expected."); break;
	}
}
