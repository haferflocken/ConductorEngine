#pragma once

#include <collection/LocklessQueue.h>
#include <file/Path.h>
#include <mem/UniquePtr.h>

#include <functional>

namespace ECS
{
class ComponentInfoFactory;
class ComponentReflector;
class EntityManager;
}

namespace Client
{
/**
 * Interface for render instances to implement so that game code can be abstracted away
 * from the details of specific rendering libraries.
 * The render instance is always instantiated from the main thread.
 *
 * A render instance is expected to render an ECS scene (entities).
 */
class IRenderInstance
{
public:
	enum class Status
	{
		Initializing,
		FailedToInitialize,
		Initialized,
		Running,
		SafeTerminated,
		ErrorTerminated,
	};

protected:
	// Protected constructor so that a subclass must be used.
	IRenderInstance() {}

public:
	virtual ~IRenderInstance() {}

	// Called from the client thread to allow any necessary initialization on it.
	virtual void InitOnClientThread() = 0;

	// Register any systems the renderer needs to the given entity manager.
	// By using ECS::Systems, game code can declare that something is renderable, and the renderer handles the rest.
	virtual void RegisterSystems(ECS::EntityManager& entityManager) = 0;

	// Returns the current status of the render instance.
	virtual Status GetStatus() const = 0;

	virtual Status Update() = 0;
};

struct InputMessage;
struct MessageToRenderInstance;

using RenderInstanceFactory = std::function<Mem::UniquePtr<IRenderInstance>(const File::Path&,
	Collection::LocklessQueue<Client::MessageToRenderInstance>&, Collection::LocklessQueue<Client::InputMessage>&)>;
}
