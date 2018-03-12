#pragma once

#include <file/Path.h>

#include <gamebase/IRenderInstance.h>

#include <vulkanrenderer/vulkanobjects/CommandBuffersObject.h>
#include <vulkanrenderer/vulkanobjects/DepthBufferObject.h>
#include <vulkanrenderer/vulkanobjects/DescriptorSetsObject.h>
#include <vulkanrenderer/vulkanobjects/DeviceObject.h>
#include <vulkanrenderer/vulkanobjects/FrameBuffersObject.h>
#include <vulkanrenderer/vulkanobjects/InstanceObject.h>
#include <vulkanrenderer/vulkanobjects/InfoObjects.h>
#include <vulkanrenderer/vulkanobjects/PipelineObject.h>
#include <vulkanrenderer/vulkanobjects/RenderPassObject.h>
#include <vulkanrenderer/vulkanobjects/ShadersObject.h>
#include <vulkanrenderer/vulkanobjects/SwapchainObject.h>
#include <vulkanrenderer/vulkanobjects/UniformBufferObject.h>
#include <vulkanrenderer/vulkanobjects/VertexBufferObject.h>
#include <vulkanrenderer/vulkanobjects/WindowObject.h>
#include <vulkanrenderer/VulkanPlatform.h>

#include <string>

namespace VulkanRenderer
{
class InstanceImpl
{
public:
	using Status = GameBase::IRenderInstance::Status;

	InstanceImpl(const char* const applicationName, const File::Path& vertexShaderFile,
		const File::Path& fragmentShaderFile);

	Status GetStatus() const { return m_status; }

	Status Update();

private:
	Status m_status;

	// The declaration order of the following members is very important, as their constructors depend on the order.
	VulkanObjects::ApplicationInfo m_applicationInfo;
	VulkanObjects::InstanceInfo m_instanceInfo;
	VulkanObjects::WindowInfo m_windowInfo;

	VulkanObjects::InstanceObject m_instance;
	VulkanObjects::WindowObject m_window;
	VulkanObjects::PhysicalDevicesInfo m_physicalDevicesInfo;

	VulkanObjects::DeviceObject m_device;
	VulkanObjects::CommandBuffersObject m_commandBuffers;

	VulkanObjects::SwapchainObject m_swapchain;

	VulkanObjects::DepthBufferObject m_depthBuffer;
	VulkanObjects::UniformBufferObject m_uniformBuffer;

	VulkanObjects::DescriptorSetsObject m_descriptorSets;
	VulkanObjects::RenderPassObject m_renderPass;
	VulkanObjects::ShadersObject m_shaders;

	VulkanObjects::FrameBuffersObject m_frameBuffers;
	VulkanObjects::VertexBufferObject m_vertexBuffer;

	VulkanObjects::PipelineObject m_pipeline;
};
}
