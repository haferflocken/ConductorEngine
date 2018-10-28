#include <renderer/RenderInstance.h>

#include <asset/AssetManager.h>

#include <client/MessageToRenderInstance.h>

#include <collection/LocklessQueue.h>

#include <ecs/ComponentInfoFactory.h>
#include <ecs/ComponentReflector.h>
#include <ecs/EntityManager.h>

#include <input/InputMessage.h>

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

enum class HandleEventResult
{
	Continue,
	EarlyOut,
};

HandleEventResult HandleSDLEvent(const SDL_Event& event, RenderInstance::Status& outStatus,
	Collection::LocklessQueue<Input::InputMessage>& inputToClientMessages)
{
	switch (event.type)
	{
	case SDL_QUIT:
	{
		outStatus = RenderInstance::Status::Terminating;

		auto message = Input::InputMessage::Make<Input::InputMessage_WindowClosed>();
		inputToClientMessages.TryPush(std::move(message));

		return HandleEventResult::EarlyOut;
	}
	case SDL_KEYDOWN:
	{
		auto message = Input::InputMessage::Make<Input::InputMessage_KeyDown>();
		auto& payload = message.Get<Input::InputMessage_KeyDown>();

		payload.m_keyCode = event.key.keysym.sym;
		strcpy_s(payload.m_keyName, sizeof(payload.m_keyName), SDL_GetKeyName(event.key.keysym.sym));

		inputToClientMessages.TryPush(std::move(message));
		return HandleEventResult::Continue;
	}
	case SDL_KEYUP:
	{
		auto message = Input::InputMessage::Make<Input::InputMessage_KeyUp>();
		auto& payload = message.Get<Input::InputMessage_KeyUp>();

		payload.m_keyCode = event.key.keysym.sym;
		strcpy_s(payload.m_keyName, sizeof(payload.m_keyName), SDL_GetKeyName(event.key.keysym.sym));

		inputToClientMessages.TryPush(std::move(message));
		return HandleEventResult::Continue;
	}
	case SDL_MOUSEMOTION:
	{
		if (event.motion.which == SDL_TOUCH_MOUSEID)
		{
			return HandleEventResult::Continue;
		}
		auto message = Input::InputMessage::Make<Input::InputMessage_MouseMotion>();
		auto& payload = message.Get<Input::InputMessage_MouseMotion>();

		payload.m_mouseX = static_cast<float>(event.motion.x) / k_width;
		payload.m_mouseY = static_cast<float>(event.motion.y) / k_height;

		inputToClientMessages.TryPush(std::move(message));
		return HandleEventResult::Continue;
	}
	case SDL_MOUSEBUTTONDOWN:
	{
		if (event.motion.which == SDL_TOUCH_MOUSEID)
		{
			return HandleEventResult::Continue;
		}
		auto message = Input::InputMessage::Make<Input::InputMessage_MouseButtonDown>();
		auto& payload = message.Get<Input::InputMessage_MouseButtonDown>();

		payload.m_mouseX = static_cast<float>(event.button.x) / k_width;
		payload.m_mouseY = static_cast<float>(event.button.y) / k_height;
		payload.m_buttonIndex = event.button.button;

		inputToClientMessages.TryPush(std::move(message));
		return HandleEventResult::Continue;
	}
	case SDL_MOUSEBUTTONUP:
	{
		if (event.motion.which == SDL_TOUCH_MOUSEID)
		{
			return HandleEventResult::Continue;
		}
		auto message = Input::InputMessage::Make<Input::InputMessage_MouseButtonUp>();
		auto& payload = message.Get<Input::InputMessage_MouseButtonUp>();

		payload.m_mouseX = static_cast<float>(event.button.x) / k_width;
		payload.m_mouseY = static_cast<float>(event.button.y) / k_height;
		payload.m_buttonIndex = event.button.button;

		inputToClientMessages.TryPush(std::move(message));
		return HandleEventResult::Continue;
	}
	default:
	{
		return HandleEventResult::Continue;
	}
	}
}
}

RenderInstance::RenderInstance(
	Asset::AssetManager& assetManager,
	Collection::LocklessQueue<Client::MessageToRenderInstance>& messagesFromClient,
	Collection::LocklessQueue<Input::InputMessage>& inputToClientMessages,
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
	// Close the window.
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
	// Shutdown bgfx.
	bgfx::shutdown();

	// Mark termination as finished.
	m_status = Status::SafeTerminated;
}

void RenderInstance::RegisterComponentTypes(ECS::ComponentReflector& componentReflector,
	ECS::ComponentInfoFactory& componentInfoFactory)
{
	componentReflector.RegisterComponentType<CameraComponent>();
	componentInfoFactory.RegisterFactoryFunction<CameraComponentInfo>();
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
	using namespace Internal_RenderInstance;

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
			if (HandleSDLEvent(event, m_status, m_inputToClientMessages) == HandleEventResult::EarlyOut)
			{
				return Status::Running;
			}
		}

		// Handle messages from the client thread.
		Client::MessageToRenderInstance message;
		while (m_messagesFromClient.TryPop(message))
		{
		}

		// Render the next frame. This will block until bgfx::frame() is called on the client thread.
		bgfx::renderFrame();

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
		AMP_FATAL_ERROR("Unknown render instance status [%d].", static_cast<int32_t>(m_status));
		return Status::ErrorTerminated;
	}
	}
}
}
