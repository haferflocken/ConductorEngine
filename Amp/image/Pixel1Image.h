#pragma once

#include <collection/BitVector.h>
#include <file/Path.h>

namespace Image
{
/**
 * A monochome image stored with 1 bit per pixel. Can be loaded from BMP files.
 */
class Pixel1Image final
{
public:
	static constexpr const char* k_fileType = ".bmp";

	static bool TryLoad(const File::Path& filePath, Pixel1Image* destination);

	Pixel1Image() = default;

	Pixel1Image(uint32_t width, uint32_t height);

	Pixel1Image(Pixel1Image&& other);
	Pixel1Image& operator=(Pixel1Image&& rhs);

	~Pixel1Image() = default;

	uint32_t GetWidth() const { return m_width; }
	uint32_t GetHeight() const { return m_height; }

	bool GetValue(uint32_t x, uint32_t y) const;
	void SetValue(uint32_t x, uint32_t y, bool v);

private:
	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	Collection::BitVector m_pixels;
};
}

// Inline implementations.
namespace Image
{
inline Pixel1Image::Pixel1Image(uint32_t width, uint32_t height)
	: m_width(width)
	, m_height(height)
	, m_pixels()
{
	m_pixels.Resize(m_width * m_height, false);
}

inline Pixel1Image::Pixel1Image(Pixel1Image&& other)
	: m_width(other.m_width)
	, m_height(other.m_height)
	, m_pixels(std::move(other.m_pixels))
{}

inline Pixel1Image& Pixel1Image::operator=(Pixel1Image&& rhs)
{
	m_width = rhs.m_width;
	m_height = rhs.m_height;
	m_pixels = std::move(rhs.m_pixels);
	return *this;
}

inline bool Pixel1Image::GetValue(uint32_t x, uint32_t y) const
{
	return m_pixels[y * m_width + x];
}

inline void Pixel1Image::SetValue(uint32_t x, uint32_t y, bool v)
{
	m_pixels[y * m_width + x] = v;
}
}
