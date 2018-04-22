#pragma once

#include <collection/VectorMap.h>
#include <util/StringHash.h>

namespace Behave
{
/**
 * A blackboard is a dictionary for data of several types, and is used to allow behaviour nodes to communicate.
 */
class Blackboard
{
public:
	Blackboard() = default;

	bool TryGetFloat(const Util::StringHash& key, float& out) const;
	
	bool TryRemove(const Util::StringHash& key);

	void Set(const Util::StringHash& key, float value);

private:
	enum class ValueType : uint8_t
	{
		Invalid,
		Float
	};

	struct TaggedValue
	{
		ValueType type;
		union
		{
			float floatValue;
		};
	};

	// A lookup of keys to values in the blackboard.
	Collection::VectorMap<Util::StringHash, TaggedValue> m_map;
};
}
