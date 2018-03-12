#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include <vulkanrenderer/VulkanPlatform.h>

namespace VulkanRenderer
{
namespace VulkanObjects
{
struct ApplicationInfo
{
	std::string name;
	uint32_t version;

	std::string engineName;
	uint32_t engineVersion;

	vk::ApplicationInfo vulkanize() const;
};

struct InstanceInfo
{
	std::vector<const char*> layerNames;
	std::vector<const char*> extensionNames;

	vk::InstanceCreateInfo vulkanize(const vk::ApplicationInfo* appInfo) const;
};

struct WindowInfo
{
	int32_t width;
	int32_t height;
};

struct PhysicalDevicesInfo
{
	std::vector<vk::PhysicalDevice> gpus;
	vk::PhysicalDevice primaryGPU;

	vk::PhysicalDeviceMemoryProperties memoryProperties;

	vk::Format colorFormat;
	vk::ColorSpaceKHR colorSpace;

	std::vector<vk::QueueFamilyProperties> queueProperties;
	uint32_t graphicsQueueFamilyIndex;
	uint32_t presentQueueFamilyIndex;

	std::vector<const char*> extensions; // Technically extensions to the logical device.
};

inline vk::ApplicationInfo ApplicationInfo::vulkanize() const
{
	return vk::ApplicationInfo()
		.setPApplicationName(name.c_str())
		.setApplicationVersion(version)
		.setPEngineName(engineName.c_str())
		.setEngineVersion(engineVersion)
		.setApiVersion(VK_API_VERSION_1_0);
}

inline vk::InstanceCreateInfo InstanceInfo::vulkanize(const vk::ApplicationInfo* appInfo) const
{
	return vk::InstanceCreateInfo()
		.setPApplicationInfo(appInfo)
		.setEnabledLayerCount(layerNames.size())
		.setPpEnabledLayerNames(!layerNames.empty() ? layerNames.data() : nullptr)
		.setEnabledExtensionCount(extensionNames.size())
		.setPpEnabledExtensionNames(extensionNames.data());
}
}
}
