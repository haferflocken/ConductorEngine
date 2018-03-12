#pragma once

#include <vulkanrenderer/vulkanobjects/InfoObjects.h>
#include <vulkanrenderer/VulkanPlatform.h>

/**
 * Contains versions of many of the utility functions in the Vulkan samples.
 */
namespace VulkanRenderer
{
namespace Utils
{
extern const vk::ComponentMapping k_colorImageComponentMapping;
extern const vk::ImageSubresourceRange k_colorSubresourceRange;
extern const vk::ImageSubresourceRange k_depthSubresourceRange;
extern const vk::Format k_depthFormat;
extern const vk::SampleCountFlagBits k_numSamples;
extern const size_t k_numDescriptorSets;

VulkanObjects::InstanceInfo MakeInstanceInfo();

VulkanObjects::WindowInfo MakeWindowInfo(const int32_t desiredWidth, const int32_t desiredHeight);

VulkanObjects::PhysicalDevicesInfo MakePhysicalDevicesInfo(const vk::Instance& instance, const vk::SurfaceKHR& surface);

uint32_t FindMemoryType(const VulkanObjects::PhysicalDevicesInfo& physicalDevicesInfo, uint32_t typeBits,
	const vk::MemoryPropertyFlags requirementsMask = vk::MemoryPropertyFlags());

void SetImageLayout(const vk::CommandBuffer& commandBuffer, const vk::Image& image,
	const vk::ImageAspectFlags aspectMask, const vk::ImageLayout oldImageLayout, const vk::ImageLayout newImageLayout);

void InitShaderCompiler();
std::vector<unsigned int> CompileShader(const vk::ShaderStageFlagBits shaderType, const std::string& shaderSource);
void FinalizeShaderCompiler();
}
}
