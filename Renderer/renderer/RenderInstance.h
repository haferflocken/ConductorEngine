#pragma once

#include <client/IRenderInstance.h>

namespace Client { struct InputMessage; struct MessageToRenderInstance; }

struct SDL_Window;

namespace Renderer
{
/**
 * A ConcurrentGame window that renders the game using BGFX.
 */
class RenderInstance final : public Client::IRenderInstance
{
public:
	explicit RenderInstance(
		Collection::LocklessQueue<Client::MessageToRenderInstance>& messagesFromClient,
		Collection::LocklessQueue<Client::InputMessage>& inputToClientMessages,
		const char* const applicationName);
	~RenderInstance();

	virtual void RegisterComponentTypes(ECS::ComponentReflector& componentReflector,
		ECS::ComponentInfoFactory& componentInfoFactory) override;
	virtual void RegisterSystems(ECS::EntityManager& entityManager) override;

	virtual Status GetStatus() const override;

	virtual Status Update() override;

private:
	Status m_status;
	Collection::LocklessQueue<Client::MessageToRenderInstance>& m_messagesFromClient;
	Collection::LocklessQueue<Client::InputMessage>& m_inputToClientMessages;

	SDL_Window* m_window;
};
}