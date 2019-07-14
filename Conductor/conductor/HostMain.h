#pragma once

#include <conductor/ApplicationErrorCode.h>
#include <conductor/IGameData.h>
#include <file/Path.h>
#include <host/HostWorld.h>

namespace Conductor
{
ApplicationErrorCode HostMain(
	const File::Path& dataDirectory,
	const File::Path& userDirectory,
	Asset::AssetManager& assetManager,
	const char* const port,
	GameDataFactory&& gameDataFactory,
	Host::HostWorld::HostFactory&& hostFactory);
}
