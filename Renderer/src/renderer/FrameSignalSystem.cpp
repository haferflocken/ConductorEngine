#include <renderer/FrameSignalSystem.h>

#include <bgfx/bgfx.h>

namespace Renderer
{
void FrameSignalSystem::Update(ECS::EntityManager& entityManager,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void()>>& deferredFunctions) const
{
	bgfx::frame();
}
}
