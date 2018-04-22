#pragma once

#include <client/ClientWorld.h>
#include <client/IRenderInstance.h>
#include <conductor/ApplicationErrorCode.h>
#include <conductor/IGameData.h>
#include <file/Path.h>
#include <host/HostWorld.h>

namespace Collection { class ProgramParameters; }

namespace Conductor
{
ApplicationErrorCode LocalClientHostMain(
	const Collection::ProgramParameters& params,
	const File::Path& dataDirectory,
	Client::RenderInstanceFactory&& renderInstanceFactory,
	GameDataFactory&& gameDataFactory,
	Client::ClientWorld::ClientFactory&& clientFactory,
	Host::HostWorld::HostFactory&& hostFactory);
}
