#include <gamebase/BaseManager.h>
//#include <gamebase/GameWorld.h>

using namespace GameBase;

void BaseManager::StartThread()
{
	m_thread = std::thread(&BaseManager::ThreadFunction, this);
}

void BaseManager::StopThread()
{
	m_thread.join();
	Teardown();
}

void BaseManager::ThreadFunction()
{
	// Initialize the thread-local environment.
	Initialize();

	// Run the manager until it stops. When it stops, the game will be triggered to stop.
	RunLoop([this](Unit::Time::Millisecond deltaTime)
	{
		return LoopBody(deltaTime);
	});
}

LoopStatus BaseManager::LoopBody(Unit::Time::Millisecond deltaTime)
{
	/*LoopStatus status;

	/*switch (m_gameWorld.GetStatus())
	{
	case GameWorld::GameStatus::Running:
	{
		// Run this manager's delegate.
		status = Update(deltaTime);
		break;
	}
	case GameWorld::GameStatus::Terminating:
	{
		// Stop running the delegate if the game terminates.
		status = LoopStatus::Stop;
		break;
	}
	default:
	{
		// GameStatus::WaitingToRun is an invalid status because this thread should not exist yet.
		// GameStatus::Terminated is an invalid status because the game should not be considered fully terminated
		// until after this thread is finished.
		status = LoopStatus::Error;
		break;
	}
	}

	m_threadStatus = status;*/
	return m_threadStatus;
}
