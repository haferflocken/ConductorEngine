#pragma once

#include <vulkanrenderer/vulkanobjects/InfoObjects.h>
#include <vulkanrenderer/VulkanPlatform.h>
#include <vulkanrenderer/VulkanUtils.h>

namespace VulkanRenderer
{
namespace VulkanObjects
{
/**
 * A descriptor tells Vulkan how to interpret the contents of a buffer or image.
 * In the case of this renderer, we only need to tell Vulkan that the uniform buffer contains the MVP matrix.
 */
class DescriptorSetsObject
{
public:
	DescriptorSetsObject(const vk::Device& device, const vk::DescriptorBufferInfo& uniformBufferInfo)
		: m_deviceRef(device)
		, m_layouts()
		, m_pool()
		, m_sets()
	{
		// Create the layouts.
		const auto layoutBinding = vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex);

		const auto layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindingCount(1)
			.setPBindings(&layoutBinding);

		m_layouts.push_back(device.createDescriptorSetLayout(layoutCreateInfo));

		if (m_layouts.size() != Utils::k_numDescriptorSets)
		{
			throw std::runtime_error("The incorrect number of descriptor set layouts were created.");
		}

		// Create a pool to allocate the layouts from, then allocate the layouts.
		const auto poolSize = vk::DescriptorPoolSize()
			.setType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1);

		const auto poolCreateInfo = vk::DescriptorPoolCreateInfo()
			.setMaxSets(1)
			.setPoolSizeCount(1)
			.setPPoolSizes(&poolSize);

		m_pool = device.createDescriptorPool(poolCreateInfo);

		const auto allocationInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(m_pool)
			.setDescriptorSetCount(Utils::k_numDescriptorSets)
			.setPSetLayouts(m_layouts.data());

		m_sets = device.allocateDescriptorSets(allocationInfo);

		// Write the buffer info into the descriptor set.
		const auto writeDescriptorSet = vk::WriteDescriptorSet()
			.setDstSet(m_sets.front())
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setPBufferInfo(&uniformBufferInfo)
			.setDstArrayElement(0)
			.setDstBinding(0);

		device.updateDescriptorSets(writeDescriptorSet, {});
	}

	~DescriptorSetsObject()
	{
		m_deviceRef.freeDescriptorSets(m_pool, m_sets);
		m_deviceRef.destroyDescriptorPool(m_pool);
		for (auto&& layout : m_layouts)
		{
			m_deviceRef.destroyDescriptorSetLayout(layout);
		}
	}

private:
	const vk::Device& m_deviceRef;

	std::vector<vk::DescriptorSetLayout> m_layouts;
	vk::DescriptorPool m_pool;
	std::vector<vk::DescriptorSet> m_sets;
};
}
}
