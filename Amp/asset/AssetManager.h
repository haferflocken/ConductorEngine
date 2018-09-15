#pragma once

#include <asset/AssetHandle.h>
#include <asset/ManagedAsset.h>
#include <collection/LinearBlockAllocator.h>
#include <collection/VectorMap.h>
#include <dev/Dev.h>
#include <file/Path.h>

#include <future>
#include <mutex>
#include <shared_mutex>

namespace Asset
{
/**
 * A type safe, thread safe, general purpose solution for asynchronously loading files from disk.
 * Loaded assets are made available through AssetHandles.
 */
class AssetManager final
{
public:
	AssetManager() = default;
	~AssetManager() = default;

	// An asset loading function either constructs in-place an asset loaded from the given file path and returns true,
	// or it does not construct an asset and returns false.
	template <typename TAsset>
	using AssetLoadingFunction = bool(*)(const File::Path&, TAsset*);

	// Register an asset type. TAsset must define static constexpr const char* k_fileType that holds the file type
	// the asset will be associated with. Only one asset type may correspond to each file type.
	template <typename TAsset>
	void RegisterAssetType(AssetLoadingFunction<TAsset> loadFn);

	// Request an asset. If the asset has already been loaded, it will be immediately available.
	// Otherwise, the asset begins to load asynchronously and becomes available once it is loaded.
	template <typename TAsset>
	AssetHandle<TAsset> RequestAsset(const File::Path& filePath);

	// Allow the asset manager to perform any book-keeping it needs to do.
	void Update();

private:
	using AssetDestructor = void(*)(void*);

	struct AssetContainer
	{
		std::mutex m_assetTypeMutex;

		void* m_loadingFunction;
		AssetDestructor m_destructorFunction;
		Collection::LinearBlockAllocator m_allocator;
		Collection::VectorMap<File::Path, void*> m_managedAssets;
		Collection::Vector<std::future<void>> m_loadingFutures;
	};
	// Shared mutex used to prevent asset type registration during RequestAsset() or Update().
	std::shared_mutex m_sharedMutex;
	// The assets and assosciated data, keyed by file type.
	Collection::VectorMap<const char*, AssetContainer> m_assetsByFileType;
};
}

// Inline implementations.
namespace Asset
{
template <typename TAsset>
inline void AssetManager::RegisterAssetType(AssetLoadingFunction<TAsset> loadFn)
{
	constexpr const char* const k_fileType = TAsset::k_fileType;
	std::unique_lock<std::shared_mutex> writeLock{ m_sharedMutex };

	Dev::FatalAssert(m_assetsByFileType.Find(k_fileType) == m_assetsByFileType.end(),
		"An asset type may not be registered multiple times.");

	AssetContainer& assetContainer = m_assetsByFileType[k_fileType];
	assetContainer.m_loadingFunction = loadFn;
	assetContainer.m_destructorFunction = [](void* managedAsset)
	{
		reinterpret_cast<ManagedAsset<TAsset>*>(managedAsset)->m_asset.TAsset();
	};
	assetContainer.m_allocator = Collection::LinearBlockAllocator::MakeFor<ManagedAsset<TAsset>>();
}

template <typename TAsset>
inline AssetHandle<TAsset> AssetManager::RequestAsset(const File::Path& filePath)
{
	constexpr const char* const k_fileType = TAsset::k_fileType;
	std::shared_lock<std::shared_mutex> readLock{ m_sharedMutex };

	Dev::FatalAssert(m_assetsByFileType.Find(k_fileType) != m_assetsByFileType.end(),
		"Cannot load an asset of an unregistered type.");

	// Obtain a lock on this asset type.
	AssetContainer& assetContainer = m_assetsByFileType[k_fileType];
	std::lock_guard guard{ assetContainer.m_assetTypeMutex };

	// If the asset has already been requested, just return it.
	auto managedAssetIter = assetContainer.m_managedAssets.Find(filePath);
	if (managedAssetIter != assetContainer.m_loadedAssets.end())
	{
		auto* const managedAsset = reinterpret_cast<ManagedAsset<TAsset>*>(managedAssetIter->second);
		++managedAsset->m_header.m_refCount;
		return AssetHandle<TAsset>(*managedAsset);
	}

	// The asset hasn't been requested before, so allocate memory for it and then load it asynchronously.
	const auto loadFn = reinterpret_cast<AssetLoadingFunction<TAsset>>(assetContainer.m_loadingFunction);
	auto* const managedAsset = reinterpret_cast<ManagedAsset<TAsset>*>(assetContainer.m_allocator.Alloc());

	managedAsset->m_header.m_status = AssetStatus::Loading;
	managedAsset->m_header.m_refCount = 1;

	assetContainer.m_managedAssets[filePath] = managedAsset;
	assetContainer.m_loadingFutures.Add(std::async(std::launch::async, [filePath, managedAsset]()
		{
			if (loadFn(filePath, &managedAsset->m_asset))
			{
				managedAsset->m_header.m_status = AssetStatus::Loaded;
			}
			else
			{
				managedAsset->m_header.m_status = AssetStatus::FailedToLoad;
			}
		}));

	return AssetHandle<TAsset>(*managedAsset);
}
}
