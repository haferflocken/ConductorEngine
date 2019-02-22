#pragma once

#include <asset/AssetManager.h>

#include <renderer/Shader.h>

namespace Renderer
{
void RegisterAssetTypes(Asset::AssetManager& assetManager)
{
	assetManager.RegisterAssetType<Shader>(".bin", Shader::TryLoad);
}

void UnregisterAssetTypes(Asset::AssetManager& assetManager)
{
	// Asset types should be unregistered in the opposite order that they are registered.
	assetManager.UnregisterAssetType<Shader>();
}
}
