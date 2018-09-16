#include <asset/AssetManager.h>

namespace Asset
{
void AssetManager::Update()
{
	std::shared_lock<std::shared_mutex> readLock{ m_sharedMutex };

	for (auto& entry : m_assetsByFileType)
	{
		AssetContainer& assetContainer = entry.second;
		std::lock_guard guard{ assetContainer.m_assetTypeMutex };

		DestroyUnreferencedAssets(assetContainer);

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

AssetManager::AssetContainer::AssetContainer(AssetContainer&& other)
{
	std::unique_lock lock{ other.m_assetTypeMutex };

	m_loadingFunction = std::move(other.m_loadingFunction);
	m_destructorFunction = std::move(other.m_destructorFunction);
	m_allocator = std::move(other.m_allocator);
	m_managedAssets = std::move(other.m_managedAssets);
	m_loadingFutures = std::move(other.m_loadingFutures);
}

AssetManager::AssetContainer& AssetManager::AssetContainer::operator=(AssetContainer&& rhs)
{
	std::unique_lock lhsLock{ m_assetTypeMutex, std::defer_lock };
	std::unique_lock rhsLock{ rhs.m_assetTypeMutex, std::defer_lock };
	std::lock(lhsLock, rhsLock);

	m_loadingFunction = std::move(rhs.m_loadingFunction);
	m_destructorFunction = std::move(rhs.m_destructorFunction);
	m_allocator = std::move(rhs.m_allocator);
	m_managedAssets = std::move(rhs.m_managedAssets);
	m_loadingFutures = std::move(rhs.m_loadingFutures);

	return *this;
}

void AssetManager::UnregisterAssetTypeInternal(const char* const fileType)
{
	std::unique_lock<std::shared_mutex> writeLock{ m_sharedMutex };

	const auto assetContainerIter = m_assetsByFileType.Find(fileType);
	Dev::FatalAssert(assetContainerIter != m_assetsByFileType.end(),
		"Cannot unregister an asset type that isn't registered.");

	AssetContainer& assetContainer = assetContainerIter->second;

	// Wait for all asset loading futures to complete.
	for (auto& future : assetContainer.m_loadingFutures)
	{
		future.wait();
	}

	// Destroy all unreferenced assets and validate that there are no assets of this type remaining.
	DestroyUnreferencedAssets(assetContainer);
	Dev::FatalAssert(assetContainer.m_managedAssets.IsEmpty(),
		"Cannot unregister an asset type that is still in use!");

	// Remove the asset container from the asset type map.
	m_assetsByFileType.TryRemove(fileType);
}

void AssetManager::DestroyUnreferencedAssets(AssetContainer& assetContainer)
{
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
}
}
