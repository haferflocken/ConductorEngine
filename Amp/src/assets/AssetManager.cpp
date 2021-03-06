#include <asset/AssetManager.h>

namespace Asset
{
void AssetManager::Update()
{
	std::shared_lock<std::shared_mutex> readLock{ m_sharedMutex };

	for (auto& entry : m_assetsByTypeHash)
	{
		AssetContainer& assetContainer = entry.second;
		std::lock_guard guard{ assetContainer.m_assetTypeMutex };

		DestroyUnreferencedAssets(assetContainer);

		// Clean up any completed asset loading futures.
		for (size_t i = 0; i < assetContainer.m_loadingFutures.Size();)
		{
			auto& future = assetContainer.m_loadingFutures[i];

			const std::future_status status = future.wait_for(std::chrono::seconds(0));
			AMP_FATAL_ASSERT(status != std::future_status::deferred, "Asset loading should always be asynchronous.");

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

	m_numLoadingFunctions = other.m_numLoadingFunctions;
	other.m_numLoadingFunctions = 0;

	for (size_t i = 0; i < m_numLoadingFunctions; ++i)
	{
		m_loadingFunctionFileTypes[i] = other.m_loadingFunctionFileTypes[i];
		m_loadingFunctions[i] = std::move(other.m_loadingFunctions[i]);
	}

	m_destructorFunction = std::move(other.m_destructorFunction);
	m_pathAllocator = std::move(other.m_pathAllocator);
	m_assetAllocator = std::move(other.m_assetAllocator);
	m_managedAssets = std::move(other.m_managedAssets);
	m_loadingFutures = std::move(other.m_loadingFutures);
}

AssetManager::AssetContainer& AssetManager::AssetContainer::operator=(AssetContainer&& rhs)
{
	std::unique_lock lhsLock{ m_assetTypeMutex, std::defer_lock };
	std::unique_lock rhsLock{ rhs.m_assetTypeMutex, std::defer_lock };
	std::lock(lhsLock, rhsLock);

	m_numLoadingFunctions = rhs.m_numLoadingFunctions;
	rhs.m_numLoadingFunctions = 0;

	for (size_t i = 0; i < m_numLoadingFunctions; ++i)
	{
		m_loadingFunctionFileTypes[i] = rhs.m_loadingFunctionFileTypes[i];
		m_loadingFunctions[i] = std::move(rhs.m_loadingFunctions[i]);
	}

	m_destructorFunction = std::move(rhs.m_destructorFunction);
	m_pathAllocator = std::move(rhs.m_pathAllocator);
	m_assetAllocator = std::move(rhs.m_assetAllocator);
	m_managedAssets = std::move(rhs.m_managedAssets);
	m_loadingFutures = std::move(rhs.m_loadingFutures);

	return *this;
}

void AssetManager::UnregisterAssetTypeInternal(const size_t typeHash)
{
	std::unique_lock<std::shared_mutex> writeLock{ m_sharedMutex };

	const auto assetContainerIter = m_assetsByTypeHash.Find(typeHash);
	AMP_FATAL_ASSERT(assetContainerIter != m_assetsByTypeHash.end(),
		"Cannot unregister an asset type that isn't registered.");

	AssetContainer& assetContainer = assetContainerIter->second;

	// Wait for all asset loading futures to complete.
	for (auto& future : assetContainer.m_loadingFutures)
	{
		future.wait();
	}

	// Destroy all unreferenced assets and validate that there are no assets of this type remaining.
	DestroyUnreferencedAssets(assetContainer);
	AMP_FATAL_ASSERT(assetContainer.m_managedAssets.IsEmpty(),
		"Cannot unregister an asset type that is still in use!");

	// Remove the asset container from the asset type map.
	m_assetsByTypeHash.TryRemove(typeHash);
}

void AssetManager::DestroyUnreferencedAssets(AssetContainer& assetContainer)
{
	assetContainer.m_managedAssets.RemoveAllMatching([&](auto& managedAssetEntry)
		{
			const ManagedAssetHeader& managedAssetHeader =
				*reinterpret_cast<const ManagedAssetHeader*>(managedAssetEntry.second);

			if (managedAssetHeader.m_refCount == 0 && managedAssetHeader.m_status != AssetStatus::Loading)
			{
				// Destroy the asset's path.
				assetContainer.m_pathAllocator.Free(const_cast<CharType*>(managedAssetEntry.first.data()));

				// Destroy the asset.
				if (managedAssetHeader.m_status == AssetStatus::Loaded)
				{
					assetContainer.m_destructorFunction(managedAssetEntry.second);
				}
				else
				{
					AMP_FATAL_ASSERT(managedAssetHeader.m_status == AssetStatus::FailedToLoad,
						"Encountered unhandled AssetStatus type [%d].",
						static_cast<int32_t>(managedAssetHeader.m_status));
				}
				assetContainer.m_assetAllocator.Free(managedAssetEntry.second);
				return true;
			}
			return false;
		});
}
}
