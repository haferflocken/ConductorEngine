#include <json/JSONTypes.h>

#include <algorithm>

namespace Internal_JSONTypes
{
using namespace JSON;

template <typename T>
bool AcceptTempl(const T& jsonValue, Visitor* visitor)
{
	const VisitorFlow controlValue = visitor->Visit(jsonValue);
	return (controlValue != VisitorFlow::Stop);
}
}

bool JSON::JSONString::Accept(Visitor* visitor) const { return Internal_JSONTypes::AcceptTempl(*this, visitor); }
bool JSON::JSONNumber::Accept(Visitor* visitor) const { return  Internal_JSONTypes::AcceptTempl(*this, visitor); }
bool JSON::JSONBoolean::Accept(Visitor* visitor) const { return  Internal_JSONTypes::AcceptTempl(*this, visitor); }
bool JSON::JSONNull::Accept(Visitor* visitor) const { return  Internal_JSONTypes::AcceptTempl(*this, visitor); }

bool JSON::JSONArray::Accept(Visitor* visitor) const
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

void JSON::JSONArray::Add(Mem::UniquePtr<JSONValue>&& value)
{
	m_array.Add(std::move(value));
}

bool JSON::JSONObject::Accept(Visitor* visitor) const
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

const JSON::JSONValue* JSON::JSONObject::FindAny(const Util::StringHash keyHash) const
{
	const auto itr = m_map.find(keyHash);
	return (itr != m_map.end()) ? itr->second : nullptr;
}

namespace Internal_JSONTypes
{
using namespace JSON;

template <typename T, ValueType type>
const T* FindWithType(const JSONObject& jsonObject, const Util::StringHash keyHash)
{
	const JSONValue* const value = jsonObject.FindAny(keyHash);
	if (value == nullptr)
	{
		return nullptr;
	}
	return (value->GetType() == type) ? static_cast<const T*>(value) : nullptr;
}
}

const JSON::JSONString* JSON::JSONObject::FindString(const Util::StringHash keyHash) const
{
	return Internal_JSONTypes::FindWithType<JSONString, ValueType::String>(*this, keyHash);
}

const JSON::JSONNumber* JSON::JSONObject::FindNumber(const Util::StringHash keyHash) const
{
	return Internal_JSONTypes::FindWithType<JSONNumber, ValueType::Number>(*this, keyHash);
}

const JSON::JSONBoolean* JSON::JSONObject::FindBoolean(const Util::StringHash keyHash) const
{
	return Internal_JSONTypes::FindWithType<JSONBoolean, ValueType::Boolean>(*this, keyHash);
}

const JSON::JSONNull* JSON::JSONObject::FindNull(const Util::StringHash keyHash) const
{
	return Internal_JSONTypes::FindWithType<JSONNull, ValueType::Null>(*this, keyHash);
}

const JSON::JSONArray* JSON::JSONObject::FindArray(const Util::StringHash keyHash) const
{
	return Internal_JSONTypes::FindWithType<JSONArray, ValueType::Array>(*this, keyHash);
}

const JSON::JSONObject* JSON::JSONObject::FindObject(const Util::StringHash keyHash) const
{
	return Internal_JSONTypes::FindWithType<JSONObject, ValueType::Object>(*this, keyHash);
}

JSON::JSONObject::Entry& JSON::JSONObject::Emplace(const std::string& key, Mem::UniquePtr<JSONValue>&& value)
{
	m_pairs.Emplace(key, std::move(value));
	Entry& entry = m_pairs.Back();
	m_map[Util::CalcHash(entry.first)] = entry.second.Get();
	return entry;
}
