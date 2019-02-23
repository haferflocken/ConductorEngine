#include <mesh/Vertex.h>

namespace Internal_Vertex
{
const std::array<const char*, static_cast<size_t>(Mesh::VertexAttribute::Count)> k_attributeStrings
{
	/* Invalid        */ "Invalid",
	/* Position       */ "Position",
	/* Normal         */ "Normal",
	/* TextureCoords0 */ "TextureCoords0",
	/* TextureCoords1 */ "TextureCoords1",
	/* Colour0 */        "Colour0",
	/* Colour1 */        "Colour1",
	/* BoneWeights */    "BoneWeights",
};
}

const char* Mesh::GetAttributeString(const VertexAttribute attribute)
{
	return Internal_Vertex::k_attributeStrings[static_cast<size_t>(attribute)];
}

Mesh::VertexAttribute Mesh::ConvertStringToVertexAttribute(const char* str)
{
	for (size_t i = 0; i < Internal_Vertex::k_attributeStrings.size(); ++i)
	{
		const char* const attributeString = Internal_Vertex::k_attributeStrings[i];
		if (strcmp(attributeString, str) == 0)
		{
			return static_cast<VertexAttribute>(i);
		}
	}
	return VertexAttribute::Invalid;
}

namespace Mesh
{
ExpandedVertexDeclaration CompactVertexDeclaration::Expand() const
{
	ExpandedVertexDeclaration expandedDeclaration;
	expandedDeclaration.m_vertexSizeInBytes = m_vertexSizeInBytes;

	uint32_t attributeOffset = 0;
	for (uint32_t i = 0; i < static_cast<uint32_t>(VertexAttribute::Count); ++i)
	{
		if ((m_attributeBitField & (1 << i)) != 0)
		{
			const size_t j = expandedDeclaration.m_numAttributes;
			++expandedDeclaration.m_numAttributes;

			const VertexAttribute attribute = static_cast<VertexAttribute>(i);
			const uint32_t attributeSize = static_cast<uint32_t>(GetAttributeSizeInBytes(attribute).GetN());

			expandedDeclaration.m_attributes[j] = attribute;
			expandedDeclaration.m_attributeSizesInBytes[j] = attributeSize;
			expandedDeclaration.m_attributeOffsets[j] = attributeOffset;

			attributeOffset += attributeSize;
		}
	}

	return expandedDeclaration;
}
}
