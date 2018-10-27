#include <scene/AnchorComponentInfo.h>

#include <json/JSONTypes.h>

namespace Scene
{
namespace Internal_AnchorComponentInfo
{
const Util::StringHash k_anchoringRadiusHash = Util::CalcHash("anchoring_radius_in_chunks");
}

const Util::StringHash AnchorComponentInfo::sk_typeHash = Util::CalcHash(AnchorComponentInfo::sk_typeName);

Mem::UniquePtr<ECS::ComponentInfo> AnchorComponentInfo::LoadFromJSON(
	const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject)
{
	const auto* const maybeAnchoringRadius = jsonObject.FindNumber(Internal_AnchorComponentInfo::k_anchoringRadiusHash);
	
	const int16_t anchoringRadiusInChunks{
		maybeAnchoringRadius != nullptr ? static_cast<int16_t>(maybeAnchoringRadius->m_number) : 1 };

	return Mem::MakeUnique<AnchorComponentInfo>(anchoringRadiusInChunks);
}

AnchorComponentInfo::AnchorComponentInfo(int16_t anchoringRadiusInChunks)
	: m_anchoringRadiusInChunks(anchoringRadiusInChunks)
{}
}
