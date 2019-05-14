#pragma once

#include <cstdint>

namespace Network
{
/**
 * Constants used in ECS transmissions.
 */
static constexpr uint32_t k_fullFrameMarker = 0xF011FA3; // F0LLFAE
static constexpr uint32_t k_deltaFrameMarker = 0xDE17AFA3; // DELTAFAE

static constexpr uint64_t k_invalidFrameIndex = UINT64_MAX;
}
