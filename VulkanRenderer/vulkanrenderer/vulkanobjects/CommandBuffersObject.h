#pragma once

#include <vulkanrenderer/vulkanobjects/InfoObjects.h>
#include <vulkanrenderer/VulkanPlatform.h>

namespace VulkanRenderer
{
namespace VulkanObjects
{
class CommandBuffersObject
{
public:
	CommandBuffersObject(const PhysicalDevicesInfo& physicalDevicesInfo, const vk::Device& deviceRef)
		: m_deviceRef(deviceRef)
		, m_commandPool()
		, m_commandBuffers()
	{
		// Create the command pool and allocate a command buffer from it.
		const vk::CommandPoolCreateInfo commandPoolCreateInfo = vk::CommandPoolCreateInfo()
			.setQueueFamilyIndex(physicalDevicesInfo.graphicsQueueFamilyIndex);

		m_commandPool = deviceRef.createCommandPool(commandPoolCreateInfo);

		const vk::CommandBufferAllocateInfo commandBufferAllocateInfo = vk::CommandBufferAllocateInfo()
			.setCommandPool(m_commandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1);

		m_commandBuffers = deviceRef.allocateCommandBuffers(commandBufferAllocateInfo);

		// Set the primary command buffer to be in the "recording" state.
		const auto commandBufferBeginInfo = vk::CommandBufferBeginInfo();
		GetPrimaryBuffer().begin(commandBufferBeginInfo);
	}

	~CommandBuffersObject()
	{
		m_deviceRef.freeCommandBuffers(m_commandPool, m_commandBuffers);
		m_deviceRef.destroyCommandPool(m_commandPool);
	}

	const vk::CommandBuffer& GetPrimaryBuffer() const { return m_commandBuffers.front(); }
	const vk::CommandBuffer& GetBuffer(const size_t index) const { return m_commandBuffers[index]; }

private:
	const vk::Device& m_deviceRef;

	// TODO: A pool and buffers should be allocated for every queue family in use.
	vk::CommandPool m_commandPool;
	std::vector<vk::CommandBuffer> m_commandBuffers;
};
}
}
