#pragma once

#include <client/ClientWorld.h>
#include <client/IRenderInstance.h>
#include <conductor/ApplicationErrorCode.h>
#include <conductor/IGameData.h>
#include <file/Path.h>
#include <host/HostWorld.h>

namespace Conductor
{
ApplicationErrorCode LocalClientHostMain(
	const File::Path& dataDirectory,
	const File::Path& userDirectory,
	Asset::AssetManager& assetManager,
	Client::RenderInstanceFactory&& renderInstanceFactory,
	GameDataFactory&& gameDataFactory,
	Client::ClientWorld::ClientFactory&& clientFactory,
	Host::HostWorld::HostFactory&& hostFactory);
}
