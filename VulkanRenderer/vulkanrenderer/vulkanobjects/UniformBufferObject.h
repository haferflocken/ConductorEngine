#pragma once

#include <vulkanrenderer/vulkanobjects/InfoObjects.h>
#include <vulkanrenderer/VulkanPlatform.h>
#include <vulkanrenderer/VulkanUtils.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace VulkanRenderer
{
namespace VulkanObjects
{
class UniformBufferObject
{
public:
	UniformBufferObject(const PhysicalDevicesInfo& physicalDevicesInfo, const vk::Device& device)
		: m_deviceRef(device)
		, m_projection()
		, m_view()
		, m_model()
		, m_clip()
		, m_modelViewProjection()
		, m_buffer()
		, m_memory()
		, m_descriptorInfo()
	{
		// Set up the camera matricies.
		m_projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

		m_view = glm::lookAt(
			glm::vec3(-5, 3, -10),	// Camera is at (-5, 3, -10) in world space.
			glm::vec3(0, 0, 0),			// and looks at the origin
			glm::vec3(0, -1, 0));		// Head is up (set to 0, -1, 0 to look upside-down)
		
		m_model = glm::mat4(1.0f);

		// Vulkan clip space has inverted Y and half Z.
		m_clip = glm::mat4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.0f, 0.0f, 0.5f, 1.0f);

		m_modelViewProjection = m_clip * m_projection * m_view * m_model;

		// Create the uniform buffer and allocate memory for it.
		const vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setUsage(vk::BufferUsageFlagBits::eUniformBuffer)
			.setSize(sizeof(m_modelViewProjection));

		m_buffer = device.createBuffer(bufferCreateInfo);

		const vk::MemoryRequirements memoryReqs = device.getBufferMemoryRequirements(m_buffer);
		const uint32_t memoryTypeIndex = Utils::FindMemoryType(physicalDevicesInfo, memoryReqs.memoryTypeBits,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		if (memoryTypeIndex == std::numeric_limits<uint32_t>::max())
		{
			throw std::runtime_error("Failed to find a memory type for the uniform buffer.");
		}
		
		const vk::MemoryAllocateInfo allocateInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(memoryReqs.size)
			.setMemoryTypeIndex(memoryTypeIndex);

		m_memory = device.allocateMemory(allocateInfo);

		// Get the uniform data into the uniform buffer memory and bind the memory to the buffer.
		void* const data = device.mapMemory(m_memory, 0, memoryReqs.size);
		memcpy(data, &m_modelViewProjection, sizeof(m_modelViewProjection));
		device.unmapMemory(m_memory);

		device.bindBufferMemory(m_buffer, m_memory, 0);

		// Store the information about the uniform buffer for the descriptor sets to use.
		m_descriptorInfo = vk::DescriptorBufferInfo()
			.setBuffer(m_buffer)
			.setOffset(0)
			.setRange(sizeof(m_modelViewProjection));
	}

	~UniformBufferObject()
	{
		m_deviceRef.destroyBuffer(m_buffer);
		m_deviceRef.freeMemory(m_memory);
	}

	const vk::DescriptorBufferInfo& GetDescriptorInfo() const { return m_descriptorInfo; }

private:
	const vk::Device& m_deviceRef;

	glm::mat4 m_projection;
	glm::mat4 m_view;
	glm::mat4 m_model;
	glm::mat4 m_clip;
	glm::mat4 m_modelViewProjection;

	vk::Buffer m_buffer;
	vk::DeviceMemory m_memory;

	vk::DescriptorBufferInfo m_descriptorInfo;
};
}
}
