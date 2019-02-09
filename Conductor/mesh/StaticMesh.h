#pragma once

#include <mesh/Vertex.h>

#include <collection/Vector.h>
#include <file/Path.h>

namespace Mesh
{
class StaticMesh
{
public:
	static constexpr const char* k_fileType = ".cms";

	static const StaticMesh k_simpleQuad;

	static bool TryLoad(const File::Path& filePath, StaticMesh* destination);
	static void SaveToFile(const File::Path& filePath, const StaticMesh& mesh);

	StaticMesh(const Collection::Vector<PosColourVertex>& vertices, Collection::Vector<uint16_t>&& triangleIndices);

	StaticMesh(const CompactVertexDeclaration& vertexDeclaration,
		Collection::Vector<uint8_t>&& vertexData,
		Collection::Vector<uint16_t>&& triangleIndices);

	const CompactVertexDeclaration& GetVertexDeclaration() const { return m_vertexDeclaration; }
	const Collection::Vector<uint8_t>& GetVertexData() const { return m_vertexData; }
	const Collection::Vector<uint16_t>& GetTriangleIndices() const { return m_triangleIndices; }

private:
	CompactVertexDeclaration m_vertexDeclaration;
	Collection::Vector<uint8_t> m_vertexData;
	Collection::Vector<uint16_t> m_triangleIndices;
};
}
