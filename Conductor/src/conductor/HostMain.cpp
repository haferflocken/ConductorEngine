#include <conductor/HostMain.h>

#include <asset/AssetManager.h>
#include <host/HostNetworkWorld.h>
#include <network/Socket.h>

#include <iostream>

Conductor::ApplicationErrorCode Conductor::HostMain(
	const Collection::ProgramParameters& params,
	const File::Path& dataDirectory,
	const char* const port,
	GameDataFactory&& gameDataFactory,
	Host::HostWorld::HostFactory&& hostFactory)
{
	// Initialize the network socket API.
	if (!Network::TryInitializeSocketAPI())
	{
		return ApplicationErrorCode::FailedToInitializeSocketAPI;
	}

	// Setup the network world and verify it is running.
	Host::HostNetworkWorld hostNetworkWorld{ port };
	if (!hostNetworkWorld.IsRunning())
	{
		Network::ShutdownSocketAPI();
		return ApplicationErrorCode::FailedToInitializeNetworkThread;
	}

	// Create an asset manager.
	Asset::AssetManager assetManager;

	// Load data files.
	Mem::UniquePtr<IGameData> gameData = gameDataFactory(assetManager, dataDirectory);

	// Create and run a host.
	Host::HostWorld hostWorld{ *gameData, hostNetworkWorld.GetClientToHostMessageQueue(), std::move(hostFactory) };
	
	// Create a thread that processes console input for as long as the network thread is running.
	std::thread consoleInputThread{ [&hostNetworkWorld]()
	{
		while (hostNetworkWorld.IsRunning())
		{
			std::string consoleInput;
			std::getline(std::cin, consoleInput);
			if (!consoleInput.empty())
			{
				hostNetworkWorld.NotifyOfConsoleInput(std::move(consoleInput));
			}
			std::this_thread::yield();
		}
	}};
	consoleInputThread.detach();

	// Block this thread until the network thread stops.
	hostNetworkWorld.WaitForShutdown();

	// Shutdown the socket API.
	Network::ShutdownSocketAPI();
	
	return ApplicationErrorCode::NoError;
}
