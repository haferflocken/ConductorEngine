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
	, m_templateParameterTypeHashes()
	, m_memberInfo()
{}

InspectorInfo::InspectorInfo(const std::type_info& typeInfo)
	: m_typeName(typeInfo.name())
	, m_typeHash{ typeInfo.hash_code() }
	, m_templateParameterTypeHashes()
	, m_memberInfo()
{
	Register(*this);
}

InspectorInfo::InspectorInfo(const std::type_info& typeInfo,
	std::initializer_list<InspectorInfoTypeHash> templateParameterTypeHashes)
	: m_typeName(typeInfo.name())
	, m_typeHash{ typeInfo.hash_code() }
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
