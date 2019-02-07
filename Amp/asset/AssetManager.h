#pragma once

#include <asset/AssetHandle.h>
#include <asset/ManagedAsset.h>
#include <collection/LinearBlockAllocator.h>
#include <collection/VectorMap.h>
#include <dev/Dev.h>
#include <file/Path.h>

#include <functional>
#include <future>
#include <mutex>
#include <shared_mutex>
#include <string_view>

namespace Asset
{
enum class LoadingMode
{
	Async = 0,
	Immediate
};

/**
 * A type safe, thread safe, general purpose solution for asynchronously loading files from disk.
 * Loaded assets are made available through AssetHandles.
 */
class AssetManager final
{
public:
	explicit AssetManager(const File::Path& assetDirectory)
		: m_assetDirectory(assetDirectory)
		, m_sharedMutex()
		, m_assetsByFileType()
	{}

	~AssetManager() = default;

	// An asset loading function either constructs in-place an asset loaded from the given file path and returns true,
	// or it does not construct an asset and returns false.
	template <typename TAsset>
	using AssetLoadingFunction = std::function<bool(const File::Path&, TAsset*)>;

	// Register an asset type. TAsset must define static constexpr const char* k_fileType that holds the file type
	// the asset will be associated with. Only one asset type may correspond to each file type.
	template <typename TAsset>
	void RegisterAssetType(AssetLoadingFunction<TAsset>&& loadFn);

	// Unregister an asset type. This will fail if any assets for that type are referenced.
	template <typename TAsset>
	void UnregisterAssetType();

	// Request an asset. If the asset has already been loaded, it will be immediately available.
	// Otherwise, the asset must be loaded. If loadingMode == LoadingMode::Async, the asset begins to load
	// asynchronously and becomes available once it is loaded. If it's LoadingMode::Immediate, this function
	// doesn't terminate until the asset is loaded.
	template <typename TAsset>
	AssetHandle<TAsset> RequestAsset(const File::Path& filePath, const LoadingMode loadingMode = LoadingMode::Async);

	// Allow the asset manager to perform any book-keeping it needs to do.
	void Update();

private:
	using AssetDestructor = void(*)(void*);
	using FilePathView = std::basic_string_view<Asset::CharType>;

	struct AssetContainer
	{
		AssetContainer() = default;

		AssetContainer(AssetContainer&&);
		AssetContainer& operator=(AssetContainer&& rhs);

		std::mutex m_assetTypeMutex;

		std::function<bool(const File::Path&, void*)> m_loadingFunction;
		AssetDestructor m_destructorFunction{ nullptr };

		// Allocates fixed-size buffers for the paths of assets in this container.
		Collection::LinearBlockAllocator m_pathAllocator;
		// Allocates ManagedAssets of the container's type.
		Collection::LinearBlockAllocator m_assetAllocator;
		// Maps asset paths to managed assets.
		// Keys are pointers into m_pathAllocator; values are pointers into m_assetAllocator.
		Collection::VectorMap<FilePathView, void*> m_managedAssets;

		// Futures used to synchronize asynchronous loading.
		Collection::Vector<std::future<void>> m_loadingFutures;
	};

	void UnregisterAssetTypeInternal(const char* const fileType);
	void DestroyUnreferencedAssets(AssetContainer& assetContainer);

private:
	// The directory within which assets are assumed to be located.
	File::Path m_assetDirectory;

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
inline void AssetManager::RegisterAssetType(AssetLoadingFunction<TAsset>&& loadFn)
{
	constexpr const char* const k_fileType = TAsset::k_fileType;
	std::unique_lock<std::shared_mutex> writeLock{ m_sharedMutex };

	AMP_FATAL_ASSERT(m_assetsByFileType.Find(k_fileType) == m_assetsByFileType.end(),
		"An asset type may not be registered multiple times.");

	AssetContainer& assetContainer = m_assetsByFileType[k_fileType];
	assetContainer.m_loadingFunction =
		[loadingFunction = std::move(loadFn)](const File::Path& filePath, void* rawAsset) mutable -> bool
	{
		return loadingFunction(filePath, reinterpret_cast<TAsset*>(rawAsset));
	};
	assetContainer.m_destructorFunction = [](void* managedAsset)
	{
		reinterpret_cast<ManagedAsset<TAsset>*>(managedAsset)->m_asset.~TAsset();
	};
	assetContainer.m_pathAllocator = Collection::LinearBlockAllocator::MakeFor<Asset::CharType[k_maxPathLength]>();
	assetContainer.m_assetAllocator = Collection::LinearBlockAllocator::MakeFor<ManagedAsset<TAsset>>();
}

template <typename TAsset>
inline void AssetManager::UnregisterAssetType()
{
	constexpr const char* const k_fileType = TAsset::k_fileType;
	UnregisterAssetTypeInternal(k_fileType);
}

template <typename TAsset>
inline AssetHandle<TAsset> AssetManager::RequestAsset(const File::Path& filePath, const LoadingMode loadingMode)
{
	constexpr const char* const k_fileType = TAsset::k_fileType;
	std::shared_lock<std::shared_mutex> readLock{ m_sharedMutex };

	AMP_FATAL_ASSERT(m_assetsByFileType.Find(k_fileType) != m_assetsByFileType.end(),
		"Cannot load an asset of an unregistered type.");

	const FilePathView filePathView{ filePath.native() };
	if (filePathView.length() >= k_maxPathLength)
	{
		AMP_LOG_WARNING("Cannot load an asset whose path exceeds the maximum path length!");
		return AssetHandle<TAsset>();
	}

	// Obtain a lock on this asset type.
	AssetContainer& assetContainer = m_assetsByFileType[k_fileType];
	std::lock_guard guard{ assetContainer.m_assetTypeMutex };

	// If the asset has already been requested, just return it.
	auto managedAssetIter = assetContainer.m_managedAssets.Find(filePathView);
	if (managedAssetIter != assetContainer.m_managedAssets.end())
	{
		auto* const managedAsset = reinterpret_cast<ManagedAsset<TAsset>*>(managedAssetIter->second);
		++managedAsset->m_header.m_refCount;
		return AssetHandle<TAsset>(*managedAsset, managedAssetIter->first.data());
	}

	// The asset hasn't been requested before, so allocate memory for it and then load it asynchronously.
	auto* const assetPath = reinterpret_cast<FilePathView::value_type*>(assetContainer.m_pathAllocator.Alloc());
	memcpy(assetPath, filePathView.data(), filePathView.length() * sizeof(Asset::CharType));
	assetPath[filePathView.length()] = '\0';

	auto* const managedAsset = reinterpret_cast<ManagedAsset<TAsset>*>(assetContainer.m_assetAllocator.Alloc());

	managedAsset->m_header.m_status = AssetStatus::Loading;
	managedAsset->m_header.m_refCount = 1;

	assetContainer.m_managedAssets[FilePathView{ assetPath }] = managedAsset;

	if (loadingMode == LoadingMode::Async)
	{
		assetContainer.m_loadingFutures.Add(std::async(std::launch::async,
			[fullPath = m_assetDirectory / filePath, loadFn = assetContainer.m_loadingFunction, managedAsset]()
		{
			// loadFn is a copy of the asset container's loading function so that a read-lock
			// doesn't have to be maintained on m_sharedMutex while the asset loads.
			if (loadFn(fullPath, &managedAsset->m_asset))
			{
				managedAsset->m_header.m_status = AssetStatus::Loaded;
			}
			else
			{
				managedAsset->m_header.m_status = AssetStatus::FailedToLoad;
			}
		}));
	}
	else
	{
		if (assetContainer.m_loadingFunction(m_assetDirectory / filePath, &managedAsset->m_asset))
		{
			managedAsset->m_header.m_status = AssetStatus::Loaded;
		}
		else
		{
			managedAsset->m_header.m_status = AssetStatus::FailedToLoad;
		}
	}

	return AssetHandle<TAsset>(*managedAsset, assetPath);
}
}
