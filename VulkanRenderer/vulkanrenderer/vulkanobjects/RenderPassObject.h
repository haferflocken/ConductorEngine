#pragma once

#include <vulkanrenderer/vulkanobjects/InfoObjects.h>
#include <vulkanrenderer/vulkanobjects/SwapchainObject.h>
#include <vulkanrenderer/VulkanPlatform.h>
#include <vulkanrenderer/VulkanUtils.h>

namespace VulkanRenderer
{
namespace VulkanObjects
{
class RenderPassObject
{
public:
	RenderPassObject(const PhysicalDevicesInfo& physicalDevicesInfo, const vk::Device& device,
		const vk::CommandBuffer& commandBuffer, const SwapchainObject& swapchainObject)
		: m_deviceRef(device)
		, m_imageAcquiredSemaphore()
		, m_currentBufferIndex(std::numeric_limits<size_t>::max())
		, m_renderPass()
	{
		// Create a semaphore which will hold back the rendering operation until the image is available.
		const auto imageAcquiredSemaphoreCreateInfo = vk::SemaphoreCreateInfo();
		m_imageAcquiredSemaphore = device.createSemaphore(imageAcquiredSemaphoreCreateInfo);

		// Acquire the next swapchain image in order to set its layout.
		const auto acquiredIndex =
			device.acquireNextImageKHR(swapchainObject.Get(), UINT64_MAX, m_imageAcquiredSemaphore, vk::Fence());
		if (acquiredIndex.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failed to acquire swapchain image when initializing the render pass.");
		}
		m_currentBufferIndex = static_cast<size_t>(acquiredIndex.value);

		// Set the layout for the color buffer to an optimal color attachment as it is currently undefined.
		const auto oldLayout = vk::ImageLayout::eUndefined;
		const auto renderTargetLayout = vk::ImageLayout::eColorAttachmentOptimal;
		Utils::SetImageLayout(commandBuffer, swapchainObject.GetBuffer(m_currentBufferIndex).image,
			vk::ImageAspectFlagBits::eColor, oldLayout, renderTargetLayout);

		// Create attachments for the render target and depth buffer.
		vk::AttachmentDescription attachments[2];
		auto& renderTargetAttachment = attachments[0];
		auto& depthBufferAttachment = attachments[1];

		renderTargetAttachment = vk::AttachmentDescription()
			.setFormat(physicalDevicesInfo.colorFormat)
			.setSamples(VulkanRenderer::Utils::k_numSamples)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(renderTargetLayout)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		depthBufferAttachment = vk::AttachmentDescription()
			.setFormat(VulkanRenderer::Utils::k_depthFormat)
			.setSamples(VulkanRenderer::Utils::k_numSamples)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		// Define the subpass, which indicates which attachements are used for what.
		const auto colorReference = vk::AttachmentReference()
			.setAttachment(0)
			.setLayout(renderTargetLayout);

		const auto depthReference = vk::AttachmentReference()
			.setAttachment(1)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		const auto subpassDescription = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(1)
			.setPColorAttachments(&colorReference)
			.setPDepthStencilAttachment(&depthReference);

		// Create the render pass, which consists of a single subpass.
		const auto renderPassCreateInfo = vk::RenderPassCreateInfo()
			.setAttachmentCount(2)
			.setPAttachments(attachments)
			.setSubpassCount(1)
			.setPSubpasses(&subpassDescription);

		m_renderPass = device.createRenderPass(renderPassCreateInfo);
	}

	~RenderPassObject()
	{
		m_deviceRef.destroyRenderPass(m_renderPass);
		m_deviceRef.destroySemaphore(m_imageAcquiredSemaphore);
	}

	const vk::RenderPass& Get() const { return m_renderPass; }

private:
	const vk::Device& m_deviceRef;

	vk::Semaphore m_imageAcquiredSemaphore;
	size_t m_currentBufferIndex;
	vk::RenderPass m_renderPass;
};
}
}
