#include <vulkanrenderer/InstanceImpl.h>

#include <vulkanrenderer/VulkanUtils.h>

#include <client/InputMessage.h>
#include <client/MessageToRenderInstance.h>
#include <collection/LocklessQueue.h>
#include <file/Path.h>

namespace Internal_InstanceImpl
{
constexpr int32_t k_desiredWidth = 1280;
constexpr int32_t k_desiredHeight = 720;
}

VulkanRenderer::InstanceImpl::InstanceImpl(
	Collection::LocklessQueue<Client::MessageToRenderInstance>& messagesFromClient,
	Collection::LocklessQueue<Client::InputMessage>& inputToClientMessages,
	const char* const applicationName, const File::Path& vertexShaderFile, const File::Path& fragmentShaderFile)
	: m_status(Status::Initializing)
	, m_messagesFromClient(messagesFromClient)
	, m_inputToClientMessages(inputToClientMessages)
	, m_applicationInfo({ applicationName, 1, "ConductorEngine", 1 })
	, m_instanceInfo(Utils::MakeInstanceInfo())
	, m_windowInfo(Utils::MakeWindowInfo(Internal_InstanceImpl::k_desiredWidth, Internal_InstanceImpl::k_desiredHeight))
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

VulkanRenderer::InstanceImpl::Status VulkanRenderer::InstanceImpl::Update()
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

			Client::InputMessage message;
			message.m_type = Client::InputMessageType::WindowClosed;

			m_inputToClientMessages.TryPush(std::move(message));
			
			return Status::SafeTerminated;
		}
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			Client::InputMessage message;
			message.m_type = (event.type == SDL_KEYDOWN)
				? Client::InputMessageType::KeyDown
				: Client::InputMessageType::KeyUp;
			message.m_key = static_cast<char>(event.key.keysym.sym);
			
			m_inputToClientMessages.TryPush(std::move(message));
			break;
		}
		default:
		{
			break;
		}
		}
	}

	Client::MessageToRenderInstance message;
	while (m_messagesFromClient.TryPop(message))
	{
		// TODO handle the messages from the client
	}

	return m_status;
}
