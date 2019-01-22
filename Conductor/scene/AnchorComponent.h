#pragma once

#include <ecs/Component.h>

namespace Scene
{
/**
 * Entities with an AnchorComponent cause the scene to load around them.
 */
class AnchorComponent final : public ECS::Component
{
public:
	static constexpr const char* k_typeName = "anchor_component";
	static const Util::StringHash k_typeHash;

	AnchorComponent(const ECS::ComponentID id, int16_t anchoringRadiusInChunks)
		: ECS::Component(id)
		, m_anchoringRadiusInChunks(anchoringRadiusInChunks)
	{}

	~AnchorComponent() {}

	int16_t m_anchoringRadiusInChunks;
};
}
