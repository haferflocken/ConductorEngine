#pragma once

#include <vulkanrenderer/VulkanInstance.h>

#include <vulkanrenderer/VulkanPlatform.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <stdint.h>
#include <limits>
#include <string>
#include <vector>

namespace VulkanRenderer
{
struct LayerProperties
{
	vk::LayerProperties properties;
	std::vector<vk::ExtensionProperties> extensions;
};

struct ShaderText
{
	ShaderText(const char* const vertexShaderText, const char* const fragmentShaderText)
		: vertexShader(vertexShaderText)
		, fragmentShader(fragmentShaderText)
	{}

	std::string vertexShader;
	std::string fragmentShader;
};

struct SwapchainBuffer
{
	vk::Image image;
	vk::ImageView view;
};

/**
 * The actual data behind a VulkanInstance. This lives in a separate header so it can just be included
 * in the VulkanInstance.cpp file and not its header.
 */
struct InstanceData
{
	explicit InstanceData(const char* const appName)
		: status(VulkanInstance::Status::Initializing)
		, applicationName(appName)
		, instance()
		, surface()
		, window(nullptr)
		, instanceLayerNames()
		, instanceExtensionNames()
		, instanceLayerProperties()
		, deviceExtensionNames()
		, deviceExtensionProperties()
		, gpus()
		, gpuProperties({})
		, device()
		, graphicsQueue()
		, presentQueue()
		, graphicsQueueFamilyIndex(std::numeric_limits<uint32_t>::max())
		, presentQueueFamilyIndex(std::numeric_limits<uint32_t>::max())
		, queueProperties()
		, queueFamilyCount(0)
		, memoryProperties({})
		, frameBuffers()
		, width(0)
		, height(0)
		, format(vk::Format::eUndefined)
		, swapchainImageCount(0)
		, swapchain()
		, swapchainBuffers()
		, currentSwapchainBuffer(std::numeric_limits<uint32_t>::max())
		, viewport()
		, scissor()
		, projection()
		, view()
		, model()
		, clip()
		, modelViewProjection()
		, commandPool()
		, commandBuffer()
		, pipelineLayout()
		, descLayout()
		, pipelineCache()
		, renderPass()
		, pipeline()
		, shaderStages()
		, descriptorPool()
		, descriptorSet()
		, depth({ vk::Format::eUndefined, nullptr, nullptr, nullptr })
		, uniformData({ nullptr, nullptr, { nullptr, 0, 0 } })
		, textureData({})
	{}

	VulkanInstance::Status status;
	std::string applicationName;

	vk::Instance instance;
	vk::SurfaceKHR surface;
	SDL_Window* window;

	std::vector<const char*> instanceLayerNames;
	std::vector<const char*> instanceExtensionNames;
	std::vector<LayerProperties> instanceLayerProperties;

	std::vector<const char*> deviceExtensionNames;
	std::vector<vk::ExtensionProperties> deviceExtensionProperties;

	std::vector<vk::PhysicalDevice> gpus;
	vk::PhysicalDeviceProperties gpuProperties;

	vk::Device device;
	vk::Queue graphicsQueue;
	vk::Queue presentQueue;
	uint32_t graphicsQueueFamilyIndex;
	uint32_t presentQueueFamilyIndex;

	std::vector<vk::QueueFamilyProperties> queueProperties;
	uint32_t queueFamilyCount;

	vk::PhysicalDeviceMemoryProperties memoryProperties;

	std::vector<vk::Framebuffer> frameBuffers;
	int32_t width;
	int32_t height;
	vk::Format format;

	uint32_t swapchainImageCount;
	vk::SwapchainKHR swapchain;
	std::vector<SwapchainBuffer> swapchainBuffers;
	uint32_t currentSwapchainBuffer;

	vk::Viewport viewport;
	vk::Rect2D scissor;

	struct
	{
		vk::Buffer buffer;
		vk::DeviceMemory memory;
		vk::DescriptorBufferInfo bufferInfo;
	} vertexBuffer;
	vk::VertexInputBindingDescription vertexInputBinding;
	vk::VertexInputAttributeDescription vertexInputAttribs[2];

	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 model;
	glm::mat4 clip;
	glm::mat4 modelViewProjection;

	vk::CommandPool commandPool;
	vk::CommandBuffer commandBuffer; // Buffer for initialization commands

	vk::PipelineLayout pipelineLayout;
	std::vector<vk::DescriptorSetLayout> descLayout;
	vk::PipelineCache pipelineCache;
	vk::RenderPass renderPass;
	vk::Pipeline pipeline;

	vk::PipelineShaderStageCreateInfo shaderStages[2];

	vk::DescriptorPool descriptorPool;
	std::vector<vk::DescriptorSet> descriptorSet;

	struct
	{
		vk::Format format;

		vk::Image image;
		vk::DeviceMemory mem;
		vk::ImageView view;
	} depth;

	struct
	{
		vk::Buffer buf;
		vk::DeviceMemory mem;
		vk::DescriptorBufferInfo bufferInfo;
	} uniformData;

	struct
	{
		vk::DescriptorImageInfo imageInfo;
	} textureData;
};
}
