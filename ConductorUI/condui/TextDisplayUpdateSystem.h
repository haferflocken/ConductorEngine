#pragma once

#include <condui/TextDisplayComponent.h>
#include <ecs/System.h>

namespace Condui
{
/**
 * Updates the strings within TextDisplayComponents using their update functions.
 */
class TextDisplayUpdateSystem final : public ECS::SystemTempl<Util::TypeList<>, Util::TypeList<TextDisplayComponent>>
{
public:
	TextDisplayUpdateSystem() = default;
	virtual ~TextDisplayUpdateSystem() {}

	void Update(
		const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const;
};
}
