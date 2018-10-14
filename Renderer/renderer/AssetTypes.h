#pragma once

#include <asset/AssetManager.h>

namespace Renderer
{
void RegisterAssetTypes(Asset::AssetManager& assetManager)
{
}

void UnregisterAssetTypes(Asset::AssetManager& assetManager)
{
	// Asset types should be unregistered in the opposite order that they are registered.
}
}
