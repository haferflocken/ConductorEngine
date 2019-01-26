#include <mem/InspectorInfo.h>

Mem::InspectorInfo Mem::MakeInspectorInfo_Internal(
	const char* name,
	std::initializer_list<const char*> memberNames,
	std::initializer_list<uint32_t> memberTypeCodes,
	std::initializer_list<size_t> memberOffsets)
{
	const size_t numMembers = memberNames.size();

	InspectorInfo info;
	info.m_name = name;
	info.m_memberInfo.Resize(static_cast<uint32_t>(numMembers));

	auto nameIter = memberNames.begin();
	auto typeCodeIter = memberTypeCodes.begin();
	auto offsetsIter = memberOffsets.begin();

	for (size_t i = 0; i < numMembers; ++i)
	{
		InspectorInfo::MemberInfo& memberInfo = info.m_memberInfo[i];
		memberInfo.m_name = *nameIter;
		memberInfo.m_typeCode = *typeCodeIter;
		memberInfo.m_offset = static_cast<uint32_t>(*offsetsIter);

		++nameIter;
		++typeCodeIter;
		++offsetsIter;
	}

	return info;
}
