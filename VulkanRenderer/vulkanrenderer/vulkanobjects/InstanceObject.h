#pragma once

#include <vulkanrenderer/vulkanobjects/InfoObjects.h>
#include <vulkanrenderer/VulkanPlatform.h>

namespace VulkanRenderer
{
namespace VulkanObjects
{
class InstanceObject
{
public:
	InstanceObject(const ApplicationInfo& applicationInfo, const InstanceInfo& instanceInfo)
	{
		const vk::ApplicationInfo appInfo = applicationInfo.vulkanize();
		const vk::InstanceCreateInfo instInfo = instanceInfo.vulkanize(&appInfo);
		m_instance = vk::createInstance(instInfo);
	}

	~InstanceObject()
	{
		m_instance.destroy();
	}

	vk::Instance& Get() { return m_instance; }

private:
	vk::Instance m_instance;
};
}
}
