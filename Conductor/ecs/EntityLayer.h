#pragma once

#include <cstdint>

namespace ECS
{
/**
 * Each entity is in exactly one layer. There are a total of 256 layers an entity can be in.
 * The top 32 layers are reserved for specific purposes.
 */
class EntityLayer final
{
	uint8_t m_value{ 0 };

public:
	EntityLayer() = default;

	explicit constexpr EntityLayer(uint8_t value)
		: m_value(value)
	{}

	~EntityLayer() = default;

	uint8_t GetValue() { return m_value; }

	bool operator==(const EntityLayer& rhs) const { return m_value == rhs.m_value; }
	bool operator!=(const EntityLayer& rhs) const { return m_value != rhs.m_value; }
	bool operator<(const EntityLayer& rhs) const { return m_value < rhs.m_value; }
};

namespace EntityLayers
{
constexpr EntityLayer k_conduiLayer{ UINT8_MAX - 31 };
}
}
