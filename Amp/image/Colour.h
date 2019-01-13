#pragma once

#include <cstdint>

namespace Image
{
struct ColourARGB
{
	ColourARGB() = default;

	constexpr ColourARGB(uint8_t _a, uint8_t _r, uint8_t _g, uint8_t _b)
		: a(_a)
		, r(_r)
		, g(_g)
		, b(_b)
	{}

	uint8_t a{ 0 };
	uint8_t r{ 0 };
	uint8_t g{ 0 };
	uint8_t b{ 0 };
};

namespace ColoursARBG
{
inline constexpr ColourARGB k_black{ 255, 0, 0, 0 };
inline constexpr ColourARGB k_white{ 255, 255, 255, 255 };
inline constexpr ColourARGB k_red{ 255, 255, 0, 0 };
inline constexpr ColourARGB k_yellow{ 255, 255, 255, 0 };
inline constexpr ColourARGB k_green{ 255, 0, 255, 0 };
inline constexpr ColourARGB k_cyan{ 255, 0, 255, 255 };
inline constexpr ColourARGB k_blue{ 255, 0, 0, 255 };
}
}
