#pragma once

#include <gamebase/Looper.h>

#include <atomic>
#include <thread>

namespace GameBase
{
class GameWorld;

class BaseManager
{
protected:
	BaseManager(const GameWorld& gameWorld)
		: m_gameWorld(gameWorld)
		, m_threadStatus(LoopStatus::Continue)
		, m_thread()
	{}

public:
	void StartThread();
	void StopThread();

	LoopStatus GetThreadStatus() const { return m_threadStatus; }

private:
	void ThreadFunction();
	LoopStatus LoopBody(Unit::Time::Millisecond deltaTime);

protected:
	// Called on m_thread before the thread's first call to Update().
	// Allows the manager to set up its thread local environment before it updates.
	virtual void Initialize() = 0;

	// Called on m_thread every step while m_thread is running.
	virtual LoopStatus Update(Unit::Time::Millisecond deltaTime) = 0;

	// Called from the GameWorld's thread after m_thread has finished.
	// Allows the manager to clean up its members before it is destructed by temporarily simulating
	// their thread-local environment. This function should always return any thread-local variables
	// to their original state before returning.
	virtual void Teardown() = 0;

	const GameWorld& m_gameWorld;
	std::atomic<LoopStatus> m_threadStatus;

private:
	std::thread m_thread;
};
}
