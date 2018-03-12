#pragma once

#include <vulkanrenderer/vulkanobjects/InfoObjects.h>
#include <vulkanrenderer/VulkanPlatform.h>

namespace VulkanRenderer
{
namespace VulkanObjects
{
class DeviceObject
{
public:
	DeviceObject(const PhysicalDevicesInfo& physicalDevicesInfo)
		: m_device()
	{
		const float queuePriorities[] = { 0.0f };

		const vk::DeviceQueueCreateInfo queueCreateInfo = vk::DeviceQueueCreateInfo()
			.setQueueFamilyIndex(physicalDevicesInfo.graphicsQueueFamilyIndex)
			.setQueueCount(1)
			.setPQueuePriorities(queuePriorities);

		const vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo()
			.setQueueCreateInfoCount(1)
			.setPQueueCreateInfos(&queueCreateInfo)
			.setEnabledExtensionCount(physicalDevicesInfo.extensions.size())
			.setPpEnabledExtensionNames(physicalDevicesInfo.extensions.data());

		m_device = physicalDevicesInfo.gpus.front().createDevice(deviceCreateInfo);
	}

	~DeviceObject()
	{
		m_device.destroy(nullptr);
	}

	const vk::Device& Get() const { return m_device; }

private:
	vk::Device m_device;
};
}
}
