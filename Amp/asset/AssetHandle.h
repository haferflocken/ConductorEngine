#pragma once

#include <asset/ManagedAsset.h>

namespace Asset
{
using CharType = wchar_t;
constexpr size_t k_maxPathLength = 64;

/**
 * Allows access to an asset managed by an AssetManager and maintains the asset's reference count.
 */
template <typename TAsset>
class AssetHandle final
{
public:
	AssetHandle() = default;
	AssetHandle(ManagedAsset<TAsset>& managedAsset, const CharType* assetPath);

	AssetHandle(const AssetHandle&);
	AssetHandle& operator=(const AssetHandle&);

	AssetHandle(AssetHandle&&);
	AssetHandle& operator=(AssetHandle&&);

	~AssetHandle();

	TAsset* TryGetAsset();
	const TAsset* TryGetAsset() const;

	const CharType* GetAssetPath() const;

	bool operator<(const AssetHandle& rhs) const;
	bool operator==(const AssetHandle& rhs) const;
	bool operator!=(const AssetHandle& rhs) const;

private:
	ManagedAsset<TAsset>* m_managedAsset{ nullptr };
	const CharType* m_assetPath{ nullptr };
};
}

// Inline implementations.
namespace Asset
{
template <typename TAsset>
inline AssetHandle<TAsset>::AssetHandle(ManagedAsset<TAsset>& managedAsset, const CharType* assetPath)
	: m_managedAsset(&managedAsset)
	, m_assetPath(assetPath)
{}

template <typename TAsset>
inline AssetHandle<TAsset>::~AssetHandle()
{
	if (m_managedAsset != nullptr)
	{
		--m_managedAsset->m_header.m_refCount;
		m_managedAsset = nullptr;
	}
}

template <typename TAsset>
inline TAsset* AssetHandle<TAsset>::TryGetAsset()
{
	return const_cast<TAsset*>(static_cast<const AssetHandle<TAsset>*>(this)->TryGetAsset());
}

template <typename TAsset>
inline const TAsset* AssetHandle<TAsset>::TryGetAsset() const
{
	if (m_managedAsset != nullptr && m_managedAsset->m_header.m_status == AssetStatus::Loaded)
	{
		return &m_managedAsset->m_asset;
	}
	return nullptr;
}


template <typename TAsset>
inline const CharType* AssetHandle<TAsset>::GetAssetPath() const
{
	return m_assetPath;
}

template <typename TAsset>
inline AssetHandle<TAsset>::AssetHandle(const AssetHandle& other)
	: m_managedAsset(other.m_managedAsset)
	, m_assetPath(other.m_assetPath)
{
	if (m_managedAsset != nullptr)
	{
		++m_managedAsset->m_header.m_refCount;
	}
}

template <typename TAsset>
inline AssetHandle<TAsset>& AssetHandle<TAsset>::operator=(const AssetHandle& rhs)
{
	if (m_managedAsset != nullptr)
	{
		--m_managedAsset->m_header.m_refCount;
	}
	m_managedAsset = rhs.m_managedAsset;
	m_assetPath = rhs.m_assetPath;
	++m_managedAsset->m_header.m_refCount;

	return *this;
}

template <typename TAsset>
inline AssetHandle<TAsset>::AssetHandle(AssetHandle&& other)
	: m_managedAsset(other.m_managedAsset)
	, m_assetPath(other.m_assetPath)
{
	other.m_managedAsset = nullptr;
	other.m_assetPath = nullptr;
}

template <typename TAsset>
inline AssetHandle<TAsset>& AssetHandle<TAsset>::operator=(AssetHandle&& rhs)
{
	if (m_managedAsset != nullptr)
	{
		--m_managedAsset->m_header.m_refCount;
	}
	m_managedAsset = rhs.m_managedAsset;
	m_assetPath = rhs.m_assetPath;
	rhs.m_managedAsset = nullptr;
	rhs.m_assetPath = nullptr;

	return *this;
}

template <typename TAsset>
inline bool AssetHandle<TAsset>::operator<(const AssetHandle& rhs) const
{
	return m_managedAsset < rhs.m_managedAsset;
}

template <typename TAsset>
inline bool AssetHandle<TAsset>::operator==(const AssetHandle& rhs) const
{
	return m_managedAsset == rhs.m_managedAsset;
}

template <typename TAsset>
inline bool AssetHandle<TAsset>::operator!=(const AssetHandle& rhs) const
{
	return m_managedAsset != rhs.m_managedAsset;
}
}
