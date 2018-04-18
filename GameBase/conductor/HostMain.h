#pragma once

#include <conductor/ApplicationErrorCode.h>
#include <conductor/IGameData.h>
#include <file/Path.h>
#include <host/HostWorld.h>

namespace Collection { class ProgramParameters; }

namespace Conductor
{
ApplicationErrorCode HostMain(
	const Collection::ProgramParameters& params,
	const File::Path& dataDirectory,
	const char* const port,
	GameDataFactory&& gameDataFactory,
	Host::HostWorld::HostFactory&& hostFactory);
}
