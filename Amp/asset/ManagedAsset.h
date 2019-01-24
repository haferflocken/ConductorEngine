#pragma once

#include <atomic>
#include <cstdint>

namespace Asset
{
enum class AssetStatus : uint8_t
{
	Loading,
	Loaded,
	FailedToLoad
};

struct ManagedAssetHeader
{
	AssetStatus m_status;
	uint8_t m_padding[3];
	std::atomic_uint32_t m_refCount;
};

/**
 * Wraps an asset with tracking data. May not be directly instantiated.
 */
template <typename TAsset>
struct ManagedAsset
{
	ManagedAsset() = delete;
	~ManagedAsset() = delete;

	ManagedAssetHeader m_header;
	TAsset m_asset;
};
}
