#pragma once

#include <renderer/MeshComponent.h>
#include <renderer/MeshComponentInfo.h>

#include <ecs/System.h>

namespace Renderer
{
class MeshSystem final : public ECS::SystemTempl<
	Util::TypeList<MeshComponent>,
	Util::TypeList<>>
{
public:
	void Update(ECS::EntityManager& entityManager,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void()>>& deferredFunctions) const;
};
}
