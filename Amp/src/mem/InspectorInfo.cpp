#include <mem/InspectorInfo.h>

#include <collection/VectorMap.h>
#include <dev/Dev.h>

namespace Mem
{
namespace Internal_InspectorInfo
{
Collection::VectorMap<size_t, const InspectorInfo*>& GetTypeInfoMap()
{
	static Collection::VectorMap<size_t, const InspectorInfo*> typeInfoMap;
	return typeInfoMap;
}
}

void InspectorInfo::Register(const InspectorInfo& info)
{
	auto& typeInfoMap = Internal_InspectorInfo::GetTypeInfoMap();
	const auto iter = typeInfoMap.Find(info.m_typeHash);
	if (iter == typeInfoMap.end())
	{
		typeInfoMap[info.m_typeHash] = &info;
	}
}

const InspectorInfo* InspectorInfo::Find(size_t typeHash)
{
	auto& typeInfoMap = Internal_InspectorInfo::GetTypeInfoMap();
	const auto iter = typeInfoMap.Find(typeHash);
	if (iter == typeInfoMap.end())
	{
		return nullptr;
	}
	return iter->second;
}

InspectorInfo::InspectorInfo(const std::type_info& typeInfo)
	: m_typeName(typeInfo.name())
	, m_typeHash(typeInfo.hash_code())
	, m_templateParameterTypeHashes()
	, m_memberInfo()
{
	Register(*this);
}

InspectorInfo::InspectorInfo(const std::type_info& typeInfo,
	std::initializer_list<size_t> templateParameterTypeHashes)
	: m_typeName(typeInfo.name())
	, m_typeHash(typeInfo.hash_code())
	, m_templateParameterTypeHashes(templateParameterTypeHashes)
	, m_memberInfo()
{
	Register(*this);
}

InspectorInfo::InspectorInfo(const std::type_info& typeInfo,
	std::initializer_list<const char*> memberNames,
	std::initializer_list<size_t> memberTypeHashes,
	std::initializer_list<size_t> memberOffsets)
	: m_typeName(typeInfo.name())
	, m_typeHash(typeInfo.hash_code())
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
