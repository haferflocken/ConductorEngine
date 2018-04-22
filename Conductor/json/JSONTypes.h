#pragma once

#include <collection/Pair.h>
#include <collection/Vector.h>
#include <mem/UniquePtr.h>
#include <util/StringHash.h>

#include <string>
#include <unordered_map>

namespace JSON
{
class JSONString;
class JSONNumber;
class JSONBoolean;
class JSONNull;
class JSONArray;
class JSONObject;

enum class VisitorFlow
{
	Visit,
	Skip,
	Stop
};

class Visitor
{
public:
	virtual ~Visitor() {}

	virtual VisitorFlow Visit(const JSONString& value) = 0;
	virtual VisitorFlow Visit(const JSONNumber& value) = 0;
	virtual VisitorFlow Visit(const JSONBoolean& value) = 0;
	virtual VisitorFlow Visit(const JSONNull& value) = 0;
	virtual VisitorFlow Visit(const JSONArray& value) = 0;
	virtual VisitorFlow Visit(const JSONObject& value) = 0;
};

enum class KeyType
{
	Root,
	String,
	Index,
};

struct JSONKey
{
	JSONKey()
		: type(KeyType::Root)
		, stringKey(nullptr)
	{}

	explicit JSONKey(const char* const key)
		: type(KeyType::String)
		, stringKey(key)
	{}

	explicit JSONKey(const size_t index)
		: type(KeyType::Index)
		, indexKey(index)
	{}

	KeyType type;
	union
	{
		const char* stringKey;
		size_t indexKey;
	};
};

enum class ValueType
{
	String,
	Number,
	Boolean,
	Null,
	Array,
	Object,
};

class JSONValue
{
public:
	virtual ValueType GetType() const = 0;
	virtual bool Accept(Visitor* visitor) const = 0;

	JSONKey m_key;
};

class JSONString : public JSONValue
{
public:
	JSONString()
		: JSONValue()
		, m_string("")
		, m_hash()
	{}

	virtual ValueType GetType() const override { return ValueType::String; }
	virtual bool Accept(Visitor* visitor) const override;

	std::string m_string;
	Util::StringHash m_hash;
};

class JSONNumber : public JSONValue
{
public:
	JSONNumber()
		: JSONValue()
		, m_number(0.0f)
	{}

	virtual ValueType GetType() const override { return ValueType::Number; }
	virtual bool Accept(Visitor* visitor) const override;

	double m_number;
};

class JSONBoolean : public JSONValue
{
public:
	JSONBoolean()
		: JSONValue()
		, m_boolean(false)
	{}

	virtual ValueType GetType() const override { return ValueType::Boolean; }
	virtual bool Accept(Visitor* visitor) const override;

	bool m_boolean;
};

class JSONNull : public JSONValue
{
public:
	JSONNull()
		: JSONValue()
	{}

	virtual ValueType GetType() const override { return ValueType::Null; }
	virtual bool Accept(Visitor* visitor) const override;
};

class JSONArray : public JSONValue
{
public:
	using iterator = Collection::Vector<Mem::UniquePtr<JSONValue>>::iterator;
	using const_iterator = Collection::Vector<Mem::UniquePtr<JSONValue>>::const_iterator;

	JSONArray()
		: JSONValue()
		, m_array()
	{}

	virtual ValueType GetType() const override { return ValueType::Array; }
	virtual bool Accept(Visitor* visitor) const override;

	size_t Size() const { return m_array.Size(); }

	void Add(Mem::UniquePtr<JSONValue>&& value);

	iterator begin() { return m_array.begin(); }
	iterator end() { return m_array.end(); }

	const_iterator begin() const { return m_array.begin(); }
	const_iterator end() const { return m_array.end(); }
	const_iterator cbegin() const { return begin(); }
	const_iterator cend() const { return end(); }

private:
	Collection::Vector<Mem::UniquePtr<JSONValue>> m_array;
};

class JSONObject : public JSONValue
{
public:
	using Entry = Collection::Pair<std::string, Mem::UniquePtr<JSONValue>>;

	using iterator = Collection::Vector<Entry>::iterator;
	using const_iterator = Collection::Vector<Entry>::const_iterator;

	JSONObject()
		: JSONValue()
		, m_pairs()
		, m_map()
	{}

	virtual ValueType GetType() const override { return ValueType::Object; }
	virtual bool Accept(Visitor* visitor) const override;

	size_t Size() const { return m_pairs.Size(); }

	const JSONValue* FindAny(const Util::StringHash keyHash) const;

	const JSONString* FindString(const Util::StringHash keyHash) const;
	const JSONNumber* FindNumber(const Util::StringHash keyHash) const;
	const JSONBoolean* FindBoolean(const Util::StringHash keyHash) const;
	const JSONNull* FindNull(const Util::StringHash keyHash) const;
	const JSONArray* FindArray(const Util::StringHash keyHash) const;
	const JSONObject* FindObject(const Util::StringHash keyHash) const;

	Entry& Emplace(const std::string& key, Mem::UniquePtr<JSONValue>&& value);

	iterator begin() { return m_pairs.begin(); }
	iterator end() { return m_pairs.end(); }

	const_iterator begin() const { return m_pairs.begin(); }
	const_iterator end() const { return m_pairs.end(); }
	const_iterator cbegin() const { return begin(); }
	const_iterator cend() const { return end(); }

private:
	// An ordered mapping of keys to objects which owns the data.
	Collection::Vector<Entry> m_pairs;
	// An unordered mapping of key hashes to objects for fast lookup.
	std::unordered_map<Util::StringHash, JSONValue*> m_map;
};
}
