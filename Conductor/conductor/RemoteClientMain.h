#pragma once

#include <client/ClientWorld.h>
#include <client/IRenderInstance.h>
#include <conductor/ApplicationErrorCode.h>
#include <conductor/IGameData.h>
#include <file/Path.h>

namespace Conductor
{
ApplicationErrorCode RemoteClientMain(
	const File::Path& dataDirectory,
	const File::Path& userDirectory,
	Asset::AssetManager& assetManager,
	const char* const hostName,
	const char* const hostPort,
	Client::RenderInstanceFactory&& renderInstanceFactory,
	GameDataFactory&& gameDataFactory,
	Client::ClientWorld::ClientFactory&& clientFactory);
}
