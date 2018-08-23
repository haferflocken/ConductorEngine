#pragma once

#include <util/StringHash.h>

namespace ECS
{
class ComponentType
{
	Util::StringHash m_typeHash;

public:
	ComponentType() = default;

	explicit ComponentType(Util::StringHash typeHash)
		: m_typeHash(typeHash)
	{}

	const Util::StringHash& GetTypeHash() const { return m_typeHash; }

	bool operator==(const ComponentType& rhs) const { return m_typeHash == rhs.m_typeHash; }
	bool operator!=(const ComponentType& rhs) const { return m_typeHash != rhs.m_typeHash; }
	bool operator<(const ComponentType& rhs) const { return m_typeHash < rhs.m_typeHash; }
};
}
