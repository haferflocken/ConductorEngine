#include <renderer/RenderInstance.h>

#include <client/InputMessage.h>
#include <client/MessageToRenderInstance.h>

#include <collection/LocklessQueue.h>

#include <ecs/ComponentInfoFactory.h>
#include <ecs/ComponentReflector.h>
#include <ecs/EntityManager.h>

#include <renderer/CameraSystem.h>
#include <renderer/FrameSignalSystem.h>
#include <renderer/MeshSystem.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

namespace Renderer
{
namespace Internal_RenderInstance
{
constexpr uint16_t k_width = 1280;
constexpr uint16_t k_height = 720;
}

RenderInstance::RenderInstance(
	Asset::AssetManager& assetManager,
	Collection::LocklessQueue<Client::MessageToRenderInstance>& messagesFromClient,
	Collection::LocklessQueue<Client::InputMessage>& inputToClientMessages,
	const char* const applicationName)
	: IRenderInstance(assetManager)
	, m_status(Status::Initializing)
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

	bgfx::renderFrame();
}

RenderInstance::~RenderInstance()
{
	SDL_DestroyWindow(m_window);
	SDL_Quit();
}

void RenderInstance::InitOnClientThread()
{
	using namespace Internal_RenderInstance;

	// Calling bgfx::init() here marks the client thread as the bgfx API thread.
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

	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xf0f0f0ff);
	
	// Mark initialization as complete.
	m_status = Status::Initialized;
}

void RenderInstance::ShutdownOnClientThread()
{
	bgfx::shutdown();
	m_status = Status::SafeTerminated;
}

void RenderInstance::RegisterComponentTypes(ECS::ComponentReflector& componentReflector,
	ECS::ComponentInfoFactory& componentInfoFactory)
{
	componentReflector.RegisterComponentType<CameraComponent>();
	componentInfoFactory.RegisterFactoryFunction<CameraComponentInfo>();

	componentReflector.RegisterComponentType<MeshComponent>();
	componentInfoFactory.RegisterFactoryFunction<MeshComponentInfo>();
}

void RenderInstance::RegisterSystems(ECS::EntityManager& entityManager)
{
	using namespace Internal_RenderInstance;
	entityManager.RegisterSystem(Mem::MakeUnique<CameraSystem>(k_width, k_height));
	entityManager.RegisterSystem(Mem::MakeUnique<MeshSystem>());
	entityManager.RegisterSystem(Mem::MakeUnique<FrameSignalSystem>());
}

RenderInstance::Status RenderInstance::GetStatus() const
{
	return m_status;
}

RenderInstance::Status RenderInstance::Update()
{
	switch (m_status)
	{
	case Status::Initializing:
	{
		// Wait until initializtion is complete before running the update.
		bgfx::renderFrame();
		return Status::Running;
	}
	case Status::Initialized:
	{
		m_status = Status::Running;
		// Fall through.
	}
	case Status::Running:
	{
		// Handle any pending SDL events.
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
			{
				m_status = Status::Terminating;

				Client::InputMessage message;
				message.m_type = Client::InputMessageType::WindowClosed;

				m_inputToClientMessages.TryPush(std::move(message));

				return Status::Running;
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

		// Handle messages from the client thread.
		Client::MessageToRenderInstance message;
		while (m_messagesFromClient.TryPop(message))
		{
		}

		// Render the next frame. This will block until bgfx::frame() is called on the client thread.
		bgfx::renderFrame();

		// Dummy draw call to ensure the view is cleared if no other draw calls are made.
		bgfx::Encoder* const encoder = bgfx::begin();
		encoder->touch(0);
		bgfx::end(encoder);

		return m_status;
	}
	case Status::Terminating:
	{
		// When terminating, wait for the client to call ShutdownOnClientThread().
		bgfx::renderFrame();
		return Status::Running;
	}
	case Status::FailedToInitialize:
	case Status::SafeTerminated:
	case Status::ErrorTerminated:
	{
		return m_status;
	}
	default:
	{
		Dev::FatalError("Unknown render instance status [%d].", static_cast<int32_t>(m_status));
		return Status::ErrorTerminated;
	}
	}
}
}
