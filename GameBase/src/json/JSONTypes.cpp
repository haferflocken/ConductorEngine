#include <json/JSONTypes.h>

#include <algorithm>

using namespace JSON;

namespace
{
template <typename T>
bool AcceptTempl(const T& jsonValue, Visitor* visitor)
{
	const VisitorFlow controlValue = visitor->Visit(jsonValue);
	return (controlValue != VisitorFlow::Stop);
}
}

bool JSONString::Accept(Visitor* visitor) const { return AcceptTempl(*this, visitor); }
bool JSONNumber::Accept(Visitor* visitor) const { return AcceptTempl(*this, visitor); }
bool JSONBoolean::Accept(Visitor* visitor) const { return AcceptTempl(*this, visitor); }
bool JSONNull::Accept(Visitor* visitor) const { return AcceptTempl(*this, visitor); }

bool JSONArray::Accept(Visitor* visitor) const
{
	VisitorFlow controlValue = visitor->Visit(*this);
	switch (controlValue)
	{
	case VisitorFlow::Skip:
	{
		return true;
	}
	case VisitorFlow::Stop:
	{
		return false;
	}
	}

	for (const auto& element : m_array)
	{
		bool keepGoing = element->Accept(visitor);
		if (!keepGoing)
		{
			return false;
		}
	}
	return true;
}

void JSONArray::Add(Mem::UniquePtr<JSONValue>&& value)
{
	m_array.Add(std::move(value));
}

bool JSONObject::Accept(Visitor* visitor) const
{
	VisitorFlow controlValue = visitor->Visit(*this);
	switch (controlValue)
	{
	case VisitorFlow::Skip:
	{
		return true;
	}
	case VisitorFlow::Stop:
	{
		return false;
	}
	}

	for (const auto& entry : m_pairs)
	{
		bool keepGoing = entry.second->Accept(visitor);
		if (!keepGoing)
		{
			return false;
		}
	}
	return true;
}

const JSONValue* JSONObject::FindAny(const Util::StringHash keyHash) const
{
	const auto itr = m_map.find(keyHash);
	return (itr != m_map.end()) ? itr->second : nullptr;
}

namespace
{
template <typename T, ValueType type>
const T* FindWithType(const JSONObject& jsonObject, const Util::StringHash keyHash)
{
	const JSONValue* const value = jsonObject.FindAny(keyHash);
	return (value->GetType() == type) ? static_cast<const T*>(value) : nullptr;
}
}

const JSONString* JSONObject::FindString(const Util::StringHash keyHash) const
{
	return FindWithType<JSONString, ValueType::String>(*this, keyHash);
}

const JSONNumber* JSONObject::FindNumber(const Util::StringHash keyHash) const
{
	return FindWithType<JSONNumber, ValueType::Number>(*this, keyHash);
}

const JSONBoolean* JSONObject::FindBoolean(const Util::StringHash keyHash) const
{
	return FindWithType<JSONBoolean, ValueType::Boolean>(*this, keyHash);
}

const JSONNull* JSONObject::FindNull(const Util::StringHash keyHash) const
{
	return FindWithType<JSONNull, ValueType::Null>(*this, keyHash);
}

const JSONArray* JSONObject::FindArray(const Util::StringHash keyHash) const
{
	return FindWithType<JSONArray, ValueType::Array>(*this, keyHash);
}

const JSONObject* JSONObject::FindObject(const Util::StringHash keyHash) const
{
	return FindWithType<JSONObject, ValueType::Object>(*this, keyHash);
}

JSONObject::Entry& JSONObject::Emplace(const std::string& key, Mem::UniquePtr<JSONValue>&& value)
{
	m_pairs.Emplace(key, std::move(value));
	Entry& entry = m_pairs.Back();
	m_map[Util::CalcHash(entry.first)] = entry.second.Get();
	return entry;
}
