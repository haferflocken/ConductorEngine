#pragma once

#include <asset/AssetHandle.h>
#include <image/Colour.h>
#include <image/Pixel1Image.h>

namespace Condui
{
struct FontInfo
{
	Asset::AssetHandle<Image::Pixel1Image> m_codePage;
	uint16_t m_characterWidthPixels;
	uint16_t m_characterHeightPixels;
	Image::ColourARGB m_textColour;
};
}
