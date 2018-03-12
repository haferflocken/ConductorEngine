#pragma once

#include <vulkanrenderer/vulkanobjects/InfoObjects.h>
#include <vulkanrenderer/VulkanPlatform.h>
#include <vulkanrenderer/VulkanUtils.h>

namespace VulkanRenderer
{
namespace VulkanObjects
{
struct SwapchainBuffer
{
	vk::Image image;
	vk::ImageView view;
};

class SwapchainObject
{
public:
	SwapchainObject(const WindowInfo& windowInfo, const vk::SurfaceKHR& surface,
		const PhysicalDevicesInfo& physicalDevicesInfo, const vk::Device& device)
		: m_deviceRef(device)
		, m_swapchain()
	{
		// Determine the dimensions and pretransform of the swapchain.
		const vk::SurfaceCapabilitiesKHR surfaceCapabilities =
			physicalDevicesInfo.primaryGPU.getSurfaceCapabilitiesKHR(surface);

		vk::Extent2D swapchainExtent;
		// Width and height are either both 0xFFFFFFF or both not.
		if (surfaceCapabilities.currentExtent.width == 0xFFFFFFFF)
		{
			const auto& minImageExtent = surfaceCapabilities.minImageExtent;
			const auto& maxImageExtent = surfaceCapabilities.maxImageExtent;

			swapchainExtent.width = std::max<uint32_t>(minImageExtent.width, windowInfo.width);
			swapchainExtent.width = std::min<uint32_t>(windowInfo.width, maxImageExtent.width);

			swapchainExtent.height = std::max<uint32_t>(minImageExtent.height, windowInfo.height);
			swapchainExtent.height = std::min<uint32_t>(windowInfo.height, maxImageExtent.height);
		}
		else
		{
			swapchainExtent = surfaceCapabilities.currentExtent;
		}

		const vk::SurfaceTransformFlagBitsKHR preTransform = (surfaceCapabilities.supportedTransforms
				& vk::SurfaceTransformFlagBitsKHR::eIdentity)
			? vk::SurfaceTransformFlagBitsKHR::eIdentity
			: surfaceCapabilities.currentTransform;

		// Get the supported present modes and use them to determine the present mode of the swapchain.
		// Mailbox is most preferred, followed by immediate and then FIFO.
		const std::vector<vk::PresentModeKHR> presentModes =
			physicalDevicesInfo.primaryGPU.getSurfacePresentModesKHR(surface);

		vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;
		for (auto&& presentMode : presentModes)
		{
			if (presentMode == vk::PresentModeKHR::eMailbox)
			{
				swapchainPresentMode = vk::PresentModeKHR::eMailbox;
				break;
			}
			if (presentMode == vk::PresentModeKHR::eImmediate)
			{
				swapchainPresentMode = vk::PresentModeKHR::eImmediate;
			}
		}

		// Pack all of that information into a create info struct.
		vk::SwapchainCreateInfoKHR swapchainCreateInfo = vk::SwapchainCreateInfoKHR()
			.setSurface(surface)
			.setImageFormat(physicalDevicesInfo.colorFormat)
			.setImageColorSpace(physicalDevicesInfo.colorSpace)
			.setMinImageCount(surfaceCapabilities.minImageCount)
			.setImageExtent(swapchainExtent)
			.setPreTransform(preTransform)
			.setPresentMode(swapchainPresentMode)
			.setImageArrayLayers(1)
			.setClipped(true)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

		// If the graphics queue family and present queue family are different, the swapchain needs to know.
		const uint32_t queueFamilyIndices[2] =
		{
			physicalDevicesInfo.graphicsQueueFamilyIndex,
			physicalDevicesInfo.presentQueueFamilyIndex
		};
		if (physicalDevicesInfo.graphicsQueueFamilyIndex != physicalDevicesInfo.presentQueueFamilyIndex)
		{
			swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
			swapchainCreateInfo.setQueueFamilyIndexCount(2);
			swapchainCreateInfo.setPQueueFamilyIndices(queueFamilyIndices);
		}

		m_swapchain = device.createSwapchainKHR(swapchainCreateInfo);
		const std::vector<vk::Image> swapchainImages = device.getSwapchainImagesKHR(m_swapchain);

		m_buffers.resize(swapchainImages.size());
		for (size_t i = 0, iEnd = swapchainImages.size(); i < iEnd; ++i)
		{
			m_buffers[i].image = swapchainImages[i];

			const vk::ImageViewCreateInfo colorImageView = vk::ImageViewCreateInfo()
				.setImage(swapchainImages[i])
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(physicalDevicesInfo.colorFormat)
				.setComponents(VulkanRenderer::Utils::k_colorImageComponentMapping)
				.setSubresourceRange(VulkanRenderer::Utils::k_colorSubresourceRange);

			m_buffers[i].view = device.createImageView(colorImageView);
		}
	}

	~SwapchainObject()
	{
		for (auto& buffer : m_buffers)
		{
			m_deviceRef.destroyImageView(buffer.view);
		}
		m_deviceRef.destroySwapchainKHR(m_swapchain);
	}

	const vk::SwapchainKHR& Get() const { return m_swapchain; }

	const SwapchainBuffer& GetBuffer(const size_t index) const { return m_buffers[index]; }

	size_t GetNumSwapchainImages() const { return m_buffers.size(); }

private:
	const vk::Device& m_deviceRef;

	vk::SwapchainKHR m_swapchain;
	std::vector<SwapchainBuffer> m_buffers;
};
}
}
