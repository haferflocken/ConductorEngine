#pragma once

#include <collection/ArrayView.h>
#include <unit/CountUnits.h>

#include <array>
#include <cstdint>

namespace Mesh
{
struct PosColourVertex
{
	float m_x;
	float m_y;
	float m_z;
	uint32_t m_abgr;
};

enum class VertexAttribute : uint32_t
{
	Invalid = 0,
	Position,
	Normal,
	TextureCoords0,
	TextureCoords1,
	Colour0,
	Colour1,
	BoneWeights,
	Count
};

static constexpr Unit::ByteCount64 GetAttributeSizeInBytes(const VertexAttribute attribute);
const char* GetAttributeString(const VertexAttribute attribute);
VertexAttribute ConvertStringToVertexAttribute(const char* str);

/**
 * Describes the layout of a vertex. Can be created from a CompactVertexDeclaration.
 */
struct ExpandedVertexDeclaration
{
	uint32_t m_vertexSizeInBytes{ 0 };
	uint32_t m_numAttributes{ 0 };
	std::array<VertexAttribute, static_cast<uint32_t>(VertexAttribute::Count)> m_attributes;
	std::array<uint32_t, static_cast<uint32_t>(VertexAttribute::Count)> m_attributeSizesInBytes;
	std::array<uint32_t, static_cast<uint32_t>(VertexAttribute::Count)> m_attributeOffsets;
};

/**
 * Describes the layout of a vertex. Supports up to 32 attribute types.
 * Attributes always occur in the order defined in VertexAttribute.
 */
class CompactVertexDeclaration
{
public:
	constexpr CompactVertexDeclaration() = default;

	explicit constexpr CompactVertexDeclaration(std::initializer_list<VertexAttribute> attributes);
	explicit CompactVertexDeclaration(const Collection::ArrayView<const VertexAttribute>& attributes);

	uint32_t GetVertexSizeInBytes() const;
	bool HasAttribute(const VertexAttribute attribute) const;

	ExpandedVertexDeclaration Expand() const;

private:
	uint32_t m_vertexSizeInBytes{ 0 };
	uint32_t m_attributeBitField{ 0 };
};
}

// Inline implementations.
inline constexpr Unit::ByteCount64 Mesh::GetAttributeSizeInBytes(const VertexAttribute attribute)
{
	const std::array<Unit::ByteCount64, static_cast<size_t>(VertexAttribute::Count)> k_attributeSizes
	{
		/* Invalid        */ Unit::ByteCount64(0),
		/* Position       */ Unit::ByteCount64(3 * sizeof(float)),
		/* Normal         */ Unit::ByteCount64(3 * sizeof(float)),
		/* TextureCoords0 */ Unit::ByteCount64(2 * sizeof(float)),
		/* TextureCoords1 */ Unit::ByteCount64(2 * sizeof(float)),
		/* Colour0 */        Unit::ByteCount64(sizeof(uint32_t)),
		/* Colour1 */        Unit::ByteCount64(sizeof(uint32_t)),
		/* BoneWeights */    Unit::ByteCount64(4 * sizeof(uint8_t)),
	};
	return k_attributeSizes[static_cast<size_t>(attribute)];
}

namespace Mesh
{
inline constexpr CompactVertexDeclaration::CompactVertexDeclaration(std::initializer_list<VertexAttribute> attributes)
{
	for (const auto& attribute : attributes)
	{
		const uint32_t bit = 1 << static_cast<uint32_t>(attribute);
		m_attributeBitField |= bit;
		m_vertexSizeInBytes += static_cast<uint32_t>(GetAttributeSizeInBytes(attribute).GetN());
	}
}

inline CompactVertexDeclaration::CompactVertexDeclaration(
	const Collection::ArrayView<const VertexAttribute>& attributes)
{
	for (const auto& attribute : attributes)
	{
		const uint32_t bit = 1 << static_cast<uint32_t>(attribute);
		m_attributeBitField |= bit;
		m_vertexSizeInBytes += static_cast<uint32_t>(GetAttributeSizeInBytes(attribute).GetN());
	}
}

inline uint32_t CompactVertexDeclaration::GetVertexSizeInBytes() const
{
	return m_vertexSizeInBytes;
}

inline bool CompactVertexDeclaration::HasAttribute(const VertexAttribute attribute) const
{
	return (m_attributeBitField & (1 << static_cast<uint32_t>(attribute))) != 0;
}
}
