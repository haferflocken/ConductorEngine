#include <vulkanrenderer/InstanceImpl.h>

#include <vulkanrenderer/VulkanUtils.h>

#include <file/Path.h>

namespace
{
constexpr int32_t k_desiredWidth = 1280;
constexpr int32_t k_desiredHeight = 720;
}

using namespace VulkanRenderer;

InstanceImpl::InstanceImpl(const char* const applicationName, const File::Path& vertexShaderFile,
	const File::Path& fragmentShaderFile)
	: m_status(Status::Initializing)
	, m_applicationInfo({ applicationName, 1, "ConcurrentGame", 1 })
	, m_instanceInfo(Utils::MakeInstanceInfo())
	, m_windowInfo(Utils::MakeWindowInfo(k_desiredWidth, k_desiredHeight))
	, m_instance(m_applicationInfo, m_instanceInfo)
	, m_window(m_instance.Get(), m_applicationInfo, m_windowInfo)
	, m_physicalDevicesInfo(Utils::MakePhysicalDevicesInfo(m_instance.Get(), m_window.GetSurface()))
	, m_device(m_physicalDevicesInfo)
	, m_commandBuffers(m_physicalDevicesInfo, m_device.Get())
	, m_swapchain(m_windowInfo, m_window.GetSurface(), m_physicalDevicesInfo, m_device.Get())
	, m_depthBuffer(m_windowInfo, m_physicalDevicesInfo, m_device.Get())
	, m_uniformBuffer(m_physicalDevicesInfo, m_device.Get())
	, m_descriptorSets(m_device.Get(), m_uniformBuffer.GetDescriptorInfo())
	, m_renderPass(m_physicalDevicesInfo, m_device.Get(), m_commandBuffers.GetPrimaryBuffer(), m_swapchain)
	, m_shaders(m_device.Get(), vertexShaderFile, fragmentShaderFile)
	, m_frameBuffers(m_windowInfo, m_device.Get(), m_swapchain, m_depthBuffer, m_renderPass)
	, m_vertexBuffer(m_physicalDevicesInfo, m_device.Get())
	, m_pipeline(m_device.Get())
{
	m_status = Status::Initialized;
}

InstanceImpl::Status InstanceImpl::Update()
{
	m_status = Status::Running;

	// Handle any pending SDL events.
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
		{
			m_status = Status::SafeTerminated;
			return Status::SafeTerminated;
		}
		default:
		{
			break;
		}
		}
	}

	// Wait for 10 ms. TODO probably just don't do this?
	SDL_Delay(10);

	return m_status;
}
