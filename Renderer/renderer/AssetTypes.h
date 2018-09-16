#pragma once

#include <renderer/mesh/StaticMesh.h>

#include <asset/AssetManager.h>

namespace Renderer
{
void RegisterAssetTypes(Asset::AssetManager& assetManager)
{
	assetManager.RegisterAssetType<Mesh::StaticMesh>(&Mesh::StaticMesh::TryLoad);
}

void UnregisterAssetTypes(Asset::AssetManager& assetManager)
{
	// Asset types should be unregistered in the opposite order that they are registered.
	assetManager.UnregisterAssetType<Mesh::StaticMesh>();
}
}
