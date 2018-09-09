#include <renderer/RenderInstance.h>

#include <client/InputMessage.h>
#include <client/MessageToRenderInstance.h>

#include <collection/LocklessQueue.h>

#include <ecs/ComponentInfoFactory.h>
#include <ecs/ComponentReflector.h>
#include <ecs/EntityManager.h>

#include <renderer/MeshComponent.h>
#include <renderer/MeshComponentInfo.h>
#include <renderer/MeshSystem.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

namespace Renderer
{
namespace Internal_RenderInstance
{
constexpr int k_width = 1280;
constexpr int k_height = 720;
}

RenderInstance::RenderInstance(
	Collection::LocklessQueue<Client::MessageToRenderInstance>& messagesFromClient,
	Collection::LocklessQueue<Client::InputMessage>& inputToClientMessages,
	const char* const applicationName)
	: m_status(Status::Initializing)
	, m_messagesFromClient(messagesFromClient)
	, m_inputToClientMessages(inputToClientMessages)
{
	using namespace Internal_RenderInstance;

	// Create an SDL window which will present a Vulkan surface.
	m_window = SDL_CreateWindow(applicationName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		k_width, k_height, SDL_WINDOW_OPENGL);
	if (m_window == nullptr)
	{
		throw std::runtime_error("Failed to create SDL window.");
	}

	SDL_SysWMinfo sdlWindowInfo;
	SDL_VERSION(&sdlWindowInfo.version);
	if (!SDL_GetWindowWMInfo(m_window, &sdlWindowInfo))
	{
		throw std::runtime_error("Failed to get SDL window info.");
	}

	// Set up BGFX.
	bgfx::PlatformData platformData;
	memset(&platformData, 0, sizeof(platformData));
	platformData.nwh = sdlWindowInfo.info.win.window;
	bgfx::setPlatformData(platformData);

	bgfx::Init init;
	init.type = bgfx::RendererType::Vulkan;
	init.vendorId = BGFX_PCI_ID_NONE;
	init.resolution.width = k_width;
	init.resolution.height = k_height;
	init.resolution.reset = BGFX_RESET_VSYNC;
	if (!bgfx::init(init))
	{
		throw std::runtime_error("Failed to initialize BGFX.");
	}

	m_status = Status::Initialized;
}

RenderInstance::~RenderInstance()
{
	bgfx::shutdown();
	SDL_DestroyWindow(m_window);
	SDL_Quit();
}

void RenderInstance::RegisterComponentTypes(ECS::ComponentReflector& componentReflector,
	ECS::ComponentInfoFactory& componentInfoFactory)
{
	componentReflector.RegisterComponentType<MeshComponent>();
	componentInfoFactory.RegisterFactoryFunction<MeshComponentInfo>();
}

void RenderInstance::RegisterSystems(ECS::EntityManager& entityManager)
{
	entityManager.RegisterSystem(Mem::MakeUnique<MeshSystem>());
}

RenderInstance::Status RenderInstance::GetStatus() const
{
	return m_status;
}

RenderInstance::Status RenderInstance::Update()
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

	// Advance to the next frame.
	bgfx::frame();

	return m_status;
}
}
