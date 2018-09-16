#pragma once

#include <conductor/IGameData.h>

namespace IslandGame
{
/**
 * Holds data that will not change over the course of the game.
 */
class IslandGameData final : public Conductor::IGameData
{
public:
	explicit IslandGameData(Asset::AssetManager& assetManager);
	virtual ~IslandGameData();
};
}
