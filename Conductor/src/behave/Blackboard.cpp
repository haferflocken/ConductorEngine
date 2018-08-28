#include <behave/Blackboard.h>

void Behave::Blackboard::GetWithDefault(
	const Util::StringHash& key,
	const AST::ExpressionResult& defaultValue,
	AST::ExpressionResult& out) const
{
	const auto* const entry = m_map.Find(key);
	if (entry == m_map.end())
	{
		out = defaultValue;
	}
	else
	{
		out = entry->second;
	}
}

bool Behave::Blackboard::TryRemove(const Util::StringHash& key)
{
	return m_map.TryRemove(key);
}

void Behave::Blackboard::Set(const Util::StringHash& key, const AST::ExpressionResult& value)
{
	m_map[key] = value;
}
