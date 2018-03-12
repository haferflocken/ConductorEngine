#pragma once

#include <vulkanrenderer/vulkanobjects/InfoObjects.h>
#include <vulkanrenderer/VulkanPlatform.h>
#include <vulkanrenderer/VulkanUtils.h>

namespace VulkanRenderer
{
namespace VulkanObjects
{
class DepthBufferObject
{
public:
	DepthBufferObject(const WindowInfo& windowInfo, const PhysicalDevicesInfo& physicalDevicesInfo,
		const vk::Device& device)
		: m_deviceRef(device)
		, m_image()
		, m_memory()
		, m_view()
	{
		// Create the image.
		const vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setFormat(VulkanRenderer::Utils::k_depthFormat)
			.setExtent(vk::Extent3D(windowInfo.width, windowInfo.height, 1))
			.setMipLevels(1)
			.setArrayLayers(1)
			.setSamples(Utils::k_numSamples)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);

		m_image = device.createImage(imageCreateInfo);

		// Allocate and bind memory for the image so we can work with it.
		const vk::MemoryRequirements memoryReq = device.getImageMemoryRequirements(m_image);
		const uint32_t memoryTypeIndex = Utils::FindMemoryType(physicalDevicesInfo, memoryReq.memoryTypeBits);
		if (memoryTypeIndex == std::numeric_limits<uint32_t>::max())
		{
			throw std::runtime_error("Failed to find a memory type index for the depth buffer.");
		}

		const vk::MemoryAllocateInfo allocateInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(memoryReq.size)
			.setMemoryTypeIndex(memoryTypeIndex);

		m_memory = device.allocateMemory(allocateInfo);
		device.bindImageMemory(m_image, m_memory, 0);

		// Create an image view to describe the depth buffer image.
		const vk::ImageViewCreateInfo viewCreateInfo = vk::ImageViewCreateInfo()
			.setImage(m_image)
			.setFormat(VulkanRenderer::Utils::k_depthFormat)
			.setComponents(Utils::k_colorImageComponentMapping)
			.setSubresourceRange(Utils::k_depthSubresourceRange)
			.setViewType(vk::ImageViewType::e2D);

		m_view = device.createImageView(viewCreateInfo);
	}

	~DepthBufferObject()
	{
		m_deviceRef.destroyImageView(m_view);
		m_deviceRef.destroyImage(m_image);
		m_deviceRef.freeMemory(m_memory); // TODO: do we need to unbind first?
	}

	const vk::ImageView& GetImageView() const { return m_view; }

private:
	const vk::Device& m_deviceRef;

	vk::Image m_image;
	vk::DeviceMemory m_memory;
	vk::ImageView m_view;
};
}
}
