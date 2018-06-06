#pragma once

#include <collection/VectorMap.h>

namespace Navigation
{
/**
 * A path cache stores paths that have already been calculated so that they don't need to be calculated again.
 */
template <typename NodeID>
class PathCache
{
	struct Key
	{
		NodeID m_start;
		NodeID m_goal;

		bool operator==(const Key& rhs) const;
		bool operator<(const Key& rhs) const;
	};

	Collection::VectorMap<Key, Collection::Vector<NodeID>> m_pathsByEndpoints{};

public:
	PathCache() = default;

	void AddPath(const NodeID& start, const NodeID& goal, Collection::Vector<NodeID>&& path);
	void ClearPath(const NodeID& start, const NodeID& goal);

	const Collection::Vector<NodeID>* FindPath(const NodeID& start, const NodeID& goal) const;
};
}

// Inline implementation.
namespace Navigation
{
template <typename NodeID>
bool PathCache<NodeID>::Key::operator==(const Key& rhs) const
{
	return m_start == rhs.m_start && m_goal == rhs.m_goal;
}

template <typename NodeID>
bool PathCache<NodeID>::Key::operator<(const Key& rhs) const
{
	if (m_start < rhs.m_start)
	{
		return true;
	}
	if (m_start == rhs.m_start)
	{
		return m_goal < rhs.m_goal;
	}
	return false;
}

template <typename NodeID>
inline void PathCache<NodeID>::AddPath(const NodeID& start, const NodeID& goal, Collection::Vector<NodeID>&& path)
{
	m_pathsByEndpoints[{ start, goal }] = std::move(path);
}

template <typename NodeID>
inline void PathCache<NodeID>::ClearPath(const NodeID& start, const NodeID& goal)
{
	m_pathsByEndpoints.TryRemove({ start, goal });
}

template <typename NodeID>
const Collection::Vector<NodeID>* PathCache<NodeID>::FindPath(const NodeID& start, const NodeID& goal) const
{
	const auto iter = m_pathsByEndpoints.Find({ start, goal });
	if (iter != m_pathsByEndpoints.end())
	{
		return &iter->second;
	}
	return nullptr;
}
}
