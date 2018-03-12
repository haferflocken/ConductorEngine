#pragma once

#include <vulkanrenderer/vulkanobjects/InfoObjects.h>
#include <vulkanrenderer/VulkanPlatform.h>
#include <vulkanrenderer/VulkanUtils.h>

#include <vulkanrenderer/cube_data.h>

#include <array>

namespace VulkanRenderer
{
namespace VulkanObjects
{
class VertexBufferObject
{
public:
	VertexBufferObject(const PhysicalDevicesInfo& physicalDevicesInfo, const vk::Device& device)
		: m_deviceRef(device)
		, m_memory()
		, m_buffer()
		, m_bindingDescription()
		, m_attributeDescriptions()
	{
		// Create a buffer.
		const auto bufferCreateInfo = vk::BufferCreateInfo()
			.setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
			.setSize(sizeof(g_vb_solid_face_colors_Data))
			.setSharingMode(vk::SharingMode::eExclusive);

		m_buffer = device.createBuffer(bufferCreateInfo);

		// Allocate memory for the buffer.
		const vk::MemoryRequirements memReqs = device.getBufferMemoryRequirements(m_buffer);
		const uint32_t memoryType = Utils::FindMemoryType(physicalDevicesInfo, memReqs.memoryTypeBits,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		
		const auto allocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(memReqs.size)
			.setMemoryTypeIndex(memoryType);

		m_memory = device.allocateMemory(allocInfo);

		// Write the vertex data into the buffer.
		void* mappedMemory = device.mapMemory(m_memory, 0, memReqs.size);
		memcpy(mappedMemory, g_vb_solid_face_colors_Data, sizeof(g_vb_solid_face_colors_Data));
		device.unmapMemory(m_memory);

		// Bind the memory to the buffer.
		device.bindBufferMemory(m_buffer, m_memory, 0);

		// Store descriptions of the vertices.
		m_bindingDescription = vk::VertexInputBindingDescription()
			.setBinding(0)
			.setInputRate(vk::VertexInputRate::eVertex)
			.setStride(sizeof(g_vb_solid_face_colors_Data[0]));

		const bool useTexture = false;
		m_attributeDescriptions =
		{
			vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(0)
				.setFormat(vk::Format::eR32G32B32A32Sfloat)
				.setOffset(0),
			vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(1)
				.setFormat(useTexture ? vk::Format::eR32G32Sfloat : vk::Format::eR32G32B32A32Sfloat)
				.setOffset(16),
		};
	}

	~VertexBufferObject()
	{
		m_deviceRef.destroyBuffer(m_buffer);
		m_deviceRef.freeMemory(m_memory);
	}

private:
	const vk::Device& m_deviceRef;
	vk::DeviceMemory m_memory;
	vk::Buffer m_buffer;

	vk::VertexInputBindingDescription m_bindingDescription;
	std::array<vk::VertexInputAttributeDescription, 2> m_attributeDescriptions;
};
}
}
