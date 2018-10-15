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

	StaticMesh(Collection::Vector<Vertex>&& vertices, Collection::Vector<uint16_t>&& triangleIndices);

	const Collection::Vector<Vertex>& GetVertices() const { return m_vertices; }
	const Collection::Vector<uint16_t>& GetTriangleIndices() const { return m_triangleIndices; }

private:
	Collection::Vector<Vertex> m_vertices;
	Collection::Vector<uint16_t> m_triangleIndices;
};
}
