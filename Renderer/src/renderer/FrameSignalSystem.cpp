#include <renderer/FrameSignalSystem.h>

#include <bgfx/bgfx.h>

namespace Renderer
{
void FrameSignalSystem::Update(const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const
{
	bgfx::frame();
}
}
