#include <renderer/MeshSystem.h>

namespace Renderer
{
void MeshSystem::Update(ECS::EntityManager& entityManager,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void()>>& deferredFunctions) const
{
	// TODO(renderer)
}
}
