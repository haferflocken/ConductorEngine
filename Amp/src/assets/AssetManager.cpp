#include <asset/AssetManager.h>

namespace Asset
{
void AssetManager::Update()
{
	for (auto& entry : m_assetsByFileType)
	{
		AssetContainer& assetContainer = entry.second;
		std::lock_guard guard{ assetContainer.m_assetTypeMutex };

		// Clean up any unreferenced assets.
		assetContainer.m_managedAssets.RemoveAllMatching([&](auto& managedAssetEntry)
			{
				const ManagedAssetHeader& managedAssetHeader =
					*reinterpret_cast<const ManagedAssetHeader*>(managedAssetEntry.second);

				if (managedAssetHeader.m_refCount == 0 && managedAssetHeader.m_status != AssetStatus::Loading)
				{
					if (managedAssetHeader.m_status == AssetStatus::Loaded)
					{
						assetContainer.m_destructorFunction(managedAssetEntry.second);
					}
					else
					{
						Dev::FatalAssert(managedAssetHeader.m_status == AssetStatus::FailedToLoad,
							"Encountered unhandled AssetStatus type [%d].",
							static_cast<int32_t>(managedAssetHeader.m_status));
					}
					assetContainer.m_allocator.Free(managedAssetEntry.second);
					return true;
				}
				return false;
			});

		// Clean up any completed asset loading futures.
		for (size_t i = 0; i < assetContainer.m_loadingFutures.Size();)
		{
			auto& future = assetContainer.m_loadingFutures[i];

			const std::future_status status = future.wait_for(std::chrono::seconds(0));
			Dev::FatalAssert(status != std::future_status::deferred, "Asset loading should always be asynchronous.");

			if (status == std::future_status::ready)
			{
				assetContainer.m_loadingFutures.SwapWithAndRemoveLast(i);
			}
			else
			{
				++i;
			}
		}
	}
}
}
