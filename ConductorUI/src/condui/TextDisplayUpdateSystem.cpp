#include <condui/TextDisplayUpdateSystem.h>

namespace Condui
{
void TextDisplayUpdateSystem::Update(
	const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const
{
	for (const auto& ecsGroup : ecsGroups)
	{
		auto& component = ecsGroup.Get<TextDisplayComponent>();
		if (component.m_stringUpdateFunction)
		{
			component.m_stringUpdateFunction(component.m_string);
		}
	}
}
}
