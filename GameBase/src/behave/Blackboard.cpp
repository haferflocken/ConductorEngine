#include <behave/Blackboard.h>

bool Behave::Blackboard::TryGetFloat(const Util::StringHash& key, float& out) const
{
	const auto* const entry = m_map.Find(key);
	if (entry == nullptr || entry->second.type != ValueType::Float)
	{
		return false;
	}

	out = entry->second.floatValue;
	return true;
}

bool Behave::Blackboard::TryRemove(const Util::StringHash& key)
{
	const auto* const entry = m_map.Find(key);
	if (entry == nullptr)
	{
		return false;
	}

	m_map.TryRemove(key);
	return true;
}

void Behave::Blackboard::Set(const Util::StringHash& key, float value)
{
	TaggedValue& taggedValue = m_map[key];
	taggedValue.type = ValueType::Float;
	taggedValue.floatValue = value;
}
