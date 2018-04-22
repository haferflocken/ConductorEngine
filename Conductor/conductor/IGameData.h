#pragma once

#include <file/Path.h>

#include <functional>

namespace Mem { template <typename T> class UniquePtr; }

namespace Conductor
{
/**
 * IGameData is the base class that any game data storage should extend.
 */
class IGameData
{
public:
	virtual ~IGameData() {}
};

using GameDataFactory = std::function<Mem::UniquePtr<IGameData>(const File::Path&)>;
}
