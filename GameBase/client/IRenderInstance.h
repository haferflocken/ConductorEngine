#pragma once

namespace Client
{
/**
 * Interface for render instances to implement so that game code can be abstracted away
 * from the details of specific rendering libraries.
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

	// Returns the current status of the render instance.
	virtual Status GetStatus() const = 0;

	virtual Status Update() = 0;
};
}
