#pragma once

#include <asset/ManagedAsset.h>

namespace Asset
{
/**
 * Allows access to an asset managed by an AssetManager and maintains the asset's reference count.
 */
template <typename TAsset>
class AssetHandle final
{
public:
	AssetHandle() = default;
	explicit AssetHandle(ManagedAsset<TAsset>& managedAsset);

	AssetHandle(const AssetHandle&);
	AssetHandle& operator=(const AssetHandle&);

	AssetHandle(AssetHandle&&);
	AssetHandle& operator=(AssetHandle&&);

	~AssetHandle();

	TAsset* TryGetAsset();
	const TAsset* TryGetAsset() const;

private:
	ManagedAsset<TAsset>* m_managedAsset{ nullptr };
};
}

// Inline implementations.
namespace Asset
{
template <typename TAsset>
inline AssetHandle<TAsset>::AssetHandle(ManagedAsset<TAsset>& managedAsset)
	: m_managedAsset(&managedAsset)
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
	if (m_managedAsset->m_status == AssetStatus::Loaded)
	{
		return &m_managedAsset->m_asset;
	}
	return nullptr;
}

template <typename TAsset>
AssetHandle<TAsset>::AssetHandle(const AssetHandle& other)
	: m_managedAsset(other.m_managedAsset)
{
	++m_managedAsset->m_header.m_refCount;
}

template <typename TAsset>
AssetHandle<TAsset>& AssetHandle<TAsset>::operator=(const AssetHandle& rhs)
{
	if (m_managedAsset != nullptr)
	{
		--m_managedAsset->m_header.m_refCount;
	}
	m_managedAsset = rhs.m_managedAsset;
	++m_managedAsset->m_header.m_refCount;

	return *this;
}

template <typename TAsset>
AssetHandle<TAsset>::AssetHandle(AssetHandle&& other)
	: m_managedAsset(other.m_managedAsset)
{
	other.m_managedAsset = nullptr;
}

template <typename TAsset>
AssetHandle<TAsset>& AssetHandle<TAsset>::operator=(AssetHandle&& rhs)
{
	if (m_managedAsset != nullptr)
	{
		--m_managedAsset->m_header.m_refCount;
	}
	m_managedAsset = rhs.m_managedAsset;
	rhs.m_managedAsset = nullptr;

	return *this;
}
}
