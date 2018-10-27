#pragma once

#include <client/IRenderInstance.h>

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
		Asset::AssetManager& assetManager,
		Collection::LocklessQueue<Client::MessageToRenderInstance>& messagesFromClient,
		Collection::LocklessQueue<Input::InputMessage>& inputToClientMessages,
		const char* const applicationName);
	~RenderInstance();

	// Register any component types the renderer needs for its ECS::Systems to run.
	static void RegisterComponentTypes(ECS::ComponentReflector& componentReflector,
		ECS::ComponentInfoFactory& componentInfoFactory);

	virtual void InitOnClientThread() override;
	virtual void ShutdownOnClientThread() override;

	virtual void RegisterSystems(ECS::EntityManager& entityManager) override;

	virtual Status GetStatus() const override;

	virtual Status Update() override;

private:
	Status m_status;
	Collection::LocklessQueue<Client::MessageToRenderInstance>& m_messagesFromClient;
	Collection::LocklessQueue<Input::InputMessage>& m_inputToClientMessages;

	SDL_Window* m_window;
};
}
