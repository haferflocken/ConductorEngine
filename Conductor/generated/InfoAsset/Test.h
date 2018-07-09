// GENERATED CODE
#pragma once


#include <collection/Vector.h>
#include <cstdint>
#include <string>

namespace InfoAsset
{
struct Test final
{
	bool m_foo{ true };
	struct Bar
	{
		struct Car
		{
			int32_t m_numWheels{ 4 };
			float m_topSpeed{ 0.000000 };
		};
		Collection::Vector<Collection::Vector<Car>> m_baz;
	};
	Bar m_bar;
};
}
// GENERATED CODE
#include <json/JSONTypes.h>

namespace InfoAsset
{
JSON::JSONObject SaveInfoInstance(const Test& infoInstance)
{
	JSON::JSONObject savedInstance;
	size_t entryID = 0;
	// root
	{
		auto value = Mem::MakeUnique<JSON::JSONArray>();
		{
			auto element = Mem::MakeUnique<JSON::JSONNumber>();
			element->m_number = entryID + 1;
			element->m_key = JSON::JSONKey(0ui64);
			value->Add(std::move(element));
		}
		{
			auto element = Mem::MakeUnique<JSON::JSONNumber>();
			element->m_number = entryID + 2;
			element->m_key = JSON::JSONKey(1ui64);
			value->Add(std::move(element));
		}
		JSON::JSONObject::Entry& entry = savedInstance.Emplace(std::to_string(entryID), std::move(value));
		entry.second->m_key = JSON::JSONKey(entry.first.data());
		++entryID;
	}
	// foo
	{
		auto value = Mem::MakeUnique<JSON::JSONBoolean>();
		value->m_boolean = infoInstance.m_foo;
		JSON::JSONObject::Entry& entry = savedInstance.Emplace(std::to_string(entryID), std::move(value));
		entry.second->m_key = JSON::JSONKey(entry.first.data());
		++entryID;
	}
	// bar
	{
		auto value = Mem::MakeUnique<JSON::JSONArray>();
		{
			auto element = Mem::MakeUnique<JSON::JSONNumber>();
			element->m_number = entryID + 1;
			element->m_key = JSON::JSONKey(0ui64);
			value->Add(std::move(element));
		}
		JSON::JSONObject::Entry& entry = savedInstance.Emplace(std::to_string(entryID), std::move(value));
		entry.second->m_key = JSON::JSONKey(entry.first.data());
		++entryID;
	}
	// baz
	{
		auto value = Mem::MakeUnique<JSON::JSONArray>();
		for (size_t i = 0, iEnd = infoInstance.m_bar.m_baz.Size(); i < iEnd; ++i)
		{
			auto element = Mem::MakeUnique<JSON::JSONNumber>();
			element->m_number = entryID + i + 1;
			element->m_key = JSON::JSONKey(i);
			value->Add(std::move(element));
		}
		JSON::JSONObject::Entry& entry = savedInstance.Emplace(std::to_string(entryID), std::move(value));
		entry.second->m_key = JSON::JSONKey(entry.first.data());
		++entryID;
	}
	for (const auto& listElement3 : infoInstance.m_bar.m_baz)
	{
		// innerList
		{
			auto value = Mem::MakeUnique<JSON::JSONArray>();
			for (size_t i = 0, iEnd = listElement3.Size(); i < iEnd; ++i)
			{
				auto element = Mem::MakeUnique<JSON::JSONNumber>();
				element->m_number = entryID + i + 1;
				element->m_key = JSON::JSONKey(i);
				value->Add(std::move(element));
			}
			JSON::JSONObject::Entry& entry = savedInstance.Emplace(std::to_string(entryID), std::move(value));
			entry.second->m_key = JSON::JSONKey(entry.first.data());
			++entryID;
		}
		for (const auto& listElement4 : listElement3)
		{
			// numWheels
			{
				auto value = Mem::MakeUnique<JSON::JSONNumber>();
				value->m_number = listElement4.m_numWheels;
				JSON::JSONObject::Entry& entry = savedInstance.Emplace(std::to_string(entryID), std::move(value));
				entry.second->m_key = JSON::JSONKey(entry.first.data());
				++entryID;
			}
			// topSpeed
			{
				auto value = Mem::MakeUnique<JSON::JSONNumber>();
				value->m_number = listElement4.m_topSpeed;
				JSON::JSONObject::Entry& entry = savedInstance.Emplace(std::to_string(entryID), std::move(value));
				entry.second->m_key = JSON::JSONKey(entry.first.data());
				++entryID;
			}
		}
	}
	return savedInstance;
}
}
