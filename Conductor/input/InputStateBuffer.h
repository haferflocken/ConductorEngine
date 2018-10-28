#pragma once

#include <cstdint>

namespace Input
{
/**
 * A record of the states that occured in a frame for the given input.
 */
struct InputStateBuffer
{
	static constexpr size_t k_capacity = 3;

	uint32_t m_count{ 0 };
	float m_values[k_capacity];
};
}
