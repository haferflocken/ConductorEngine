#include <ecs/ActorComponentGroupVector.h>

#include <dev/Dev.h>

#include <array>

void ECS::ActorComponentGroupVector::Add(const Collection::Vector<size_t>& indices)
{
	Dev::FatalAssert(indices.Size() == m_groupSize,
		"Can only add actor component groups with the correct group size.");
	
	for (const auto& index : indices)
	{
		m_data.Add(index);
	}
}

namespace Internal_ActorComponentGroupVector
{
template <uint32_t GroupSize>
void SortGroups(Collection::Vector<size_t>& data, const uint32_t numGroups)
{
	using ArrayType = std::array<size_t, GroupSize>;

	Collection::ArrayView<ArrayType> view{ reinterpret_cast<ArrayType*>(&data[0]), numGroups };

	std::sort(view.begin(), view.end(), [](const ArrayType& lhs, const ArrayType& rhs)
	{
		return lhs[0] < rhs[0];
	});
}
}

void ECS::ActorComponentGroupVector::Sort()
{
	using namespace Internal_ActorComponentGroupVector;

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
	default: Dev::FatalError("Encountered a component group with more elements than expected."); break;
	}
}
