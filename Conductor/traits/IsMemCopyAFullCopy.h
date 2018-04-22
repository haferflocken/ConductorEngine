#pragma once

#include <cstdint>

namespace Traits
{
/**
 * Traits class which indicates if memcpy will fully copy a value of a type.
 * Defaults to false, but is specialized to true for numeric types.
 */
template <typename T>
struct IsMemCopyAFullCopy { static constexpr bool value = false; };

template <> struct IsMemCopyAFullCopy<bool> { static constexpr bool value = true; };

template <> struct IsMemCopyAFullCopy<uint8_t> { static constexpr bool value = true; };
template <> struct IsMemCopyAFullCopy<uint16_t> { static constexpr bool value = true; };
template <> struct IsMemCopyAFullCopy<uint32_t> { static constexpr bool value = true; };
template <> struct IsMemCopyAFullCopy<uint64_t> { static constexpr bool value = true; };

template <> struct IsMemCopyAFullCopy<int8_t> { static constexpr bool value = true; };
template <> struct IsMemCopyAFullCopy<int16_t> { static constexpr bool value = true; };
template <> struct IsMemCopyAFullCopy<int32_t> { static constexpr bool value = true; };
template <> struct IsMemCopyAFullCopy<int64_t> { static constexpr bool value = true; };

template <> struct IsMemCopyAFullCopy<float> { static constexpr bool value = true; };
template <> struct IsMemCopyAFullCopy<double> { static constexpr bool value = true; };
}
