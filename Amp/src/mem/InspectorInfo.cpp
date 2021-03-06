#include <mem/InspectorInfo.h>

#include <collection/VectorMap.h>
#include <dev/Dev.h>

namespace Mem
{
namespace Internal_InspectorInfo
{
Collection::VectorMap<InspectorInfoTypeHash, InspectorInfo>& GetTypeInfoMap()
{
	static Collection::VectorMap<InspectorInfoTypeHash, InspectorInfo> typeInfoMap;
	return typeInfoMap;
}

size_t IndexOfChar(const char c, const char* str, size_t resultIfNotPresent)
{
	const char* const cPtr = strchr(str, c);
	if (cPtr == nullptr)
	{
		return resultIfNotPresent;
	}
	return static_cast<size_t>(cPtr - str);
}
}

bool InspectorInfoTypeHash::operator==(const InspectorInfoTypeHash& rhs) const
{
	return m_value == rhs.m_value;
}

bool InspectorInfoTypeHash::operator!=(const InspectorInfoTypeHash& rhs) const
{
	return m_value != rhs.m_value;
}

bool InspectorInfoTypeHash::operator<(const InspectorInfoTypeHash& rhs) const
{
	return m_value < rhs.m_value;
}

void InspectorInfo::Register(const InspectorInfo& info)
{
	auto& typeInfoMap = Internal_InspectorInfo::GetTypeInfoMap();
	const auto iter = typeInfoMap.Find(info.m_typeHash);
	if (iter == typeInfoMap.end())
	{
		typeInfoMap[info.m_typeHash] = info;
	}
	else
	{
		// Only member info is ever updated when registering InspectorInfo.
		InspectorInfo& existingInfo = iter->second;
		if (existingInfo.m_memberInfo.IsEmpty() && !info.m_memberInfo.IsEmpty())
		{
			existingInfo.m_memberInfo = info.m_memberInfo;
		}
	}
}

const InspectorInfo* InspectorInfo::Find(InspectorInfoTypeHash typeHash)
{
	auto& typeInfoMap = Internal_InspectorInfo::GetTypeInfoMap();
	const auto iter = typeInfoMap.Find(typeHash);
	if (iter == typeInfoMap.end())
	{
		return nullptr;
	}
	return &iter->second;
}

InspectorInfo::InspectorInfo()
	: m_typeName(nullptr)
	, m_typeHash{ 0 }
	, m_templateTypeNameLength(0)
	, m_templateParameterTypeHashes()
	, m_memberInfo()
{}

InspectorInfo::InspectorInfo(const std::type_info& typeInfo)
	: m_typeName(typeInfo.name())
	, m_typeHash{ typeInfo.hash_code() }
	, m_templateTypeNameLength(0)
	, m_templateParameterTypeHashes()
	, m_memberInfo()
{
	Register(*this);
}

InspectorInfo::InspectorInfo(const std::type_info& typeInfo,
	std::initializer_list<InspectorInfoTypeHash> templateParameterTypeHashes)
	: m_typeName(typeInfo.name())
	, m_typeHash{ typeInfo.hash_code() }
	, m_templateTypeNameLength(Internal_InspectorInfo::IndexOfChar('<', m_typeName, 0))
	, m_templateParameterTypeHashes()
	, m_memberInfo()
{
	m_templateParameterTypeHashes.Resize(static_cast<uint32_t>(templateParameterTypeHashes.size()));
	auto templateParameterTypeHashIter = templateParameterTypeHashes.begin();
	for (size_t i = 0; i < templateParameterTypeHashes.size(); ++i)
	{
		m_templateParameterTypeHashes[i] = *templateParameterTypeHashIter;
		++templateParameterTypeHashIter;
	}

	Register(*this);
}

InspectorInfo::InspectorInfo(const std::type_info& typeInfo,
	std::initializer_list<const char*> memberNames,
	std::initializer_list<InspectorInfoTypeHash> memberTypeHashes,
	std::initializer_list<size_t> memberOffsets)
	: m_typeName(typeInfo.name())
	, m_typeHash{ typeInfo.hash_code() }
	, m_templateTypeNameLength(0)
	, m_templateParameterTypeHashes()
	, m_memberInfo()
{
	m_memberInfo.Resize(static_cast<uint32_t>(memberNames.size()));
	auto memberNameIter = memberNames.begin();
	auto memberTypeIter = memberTypeHashes.begin();
	auto memberOffsetIter = memberOffsets.begin();
	for (size_t i = 0; i < memberNames.size(); ++i)
	{
		auto& memberInfo = m_memberInfo[i];
		memberInfo.m_name = *memberNameIter;
		memberInfo.m_typeHash = *memberTypeIter;
		memberInfo.m_offset = *memberOffsetIter;
		++memberNameIter;
		++memberTypeIter;
		++memberOffsetIter;
	}

	Register(*this);
}
}
