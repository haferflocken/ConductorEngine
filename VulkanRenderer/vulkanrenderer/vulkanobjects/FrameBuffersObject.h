#pragma once

#include <vulkanrenderer/VulkanPlatform.h>
#include <vulkanrenderer/vulkanobjects/DepthBufferObject.h>
#include <vulkanrenderer/vulkanobjects/RenderPassObject.h>
#include <vulkanrenderer/vulkanobjects/SwapchainObject.h>

namespace VulkanRenderer
{
namespace VulkanObjects
{
class FrameBuffersObject
{
public:
	FrameBuffersObject(const WindowInfo& windowInfo, const vk::Device& device, const SwapchainObject& swapchainObject,
		const DepthBufferObject& depthBufferObject, const RenderPassObject& renderPassObject)
		: m_deviceRef(device)
		, m_frameBuffers()
	{
		// Make a frame buffer for each swapchain image.
		const vk::ImageView& depthImageView = depthBufferObject.GetImageView();

		const size_t numSwapchainImages = swapchainObject.GetNumSwapchainImages();
		for (size_t i = 0; i < numSwapchainImages; ++i)
		{
			const vk::ImageView& swapchainImageView = swapchainObject.GetBuffer(i).view;
			const vk::ImageView attachments[2] = { swapchainImageView, depthImageView };

			const auto fbCreateInfo = vk::FramebufferCreateInfo()
				.setRenderPass(renderPassObject.Get())
				.setAttachmentCount(2)
				.setPAttachments(attachments)
				.setWidth(windowInfo.width)
				.setHeight(windowInfo.height)
				.setLayers(1);

			m_frameBuffers.push_back(device.createFramebuffer(fbCreateInfo));
		}
	}

	~FrameBuffersObject()
	{
		for (auto&& fB : m_frameBuffers)
		{
			m_deviceRef.destroyFramebuffer(fB);
		}
	}

private:
	const vk::Device& m_deviceRef;
	std::vector<vk::Framebuffer> m_frameBuffers;
};
}
}
