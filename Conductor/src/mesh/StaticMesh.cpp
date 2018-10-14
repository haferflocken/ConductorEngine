#include <mesh/StaticMesh.h>

namespace Mesh
{
bool StaticMesh::TryLoad(const File::Path& filePath, StaticMesh* destination)
{
	Collection::Vector<Vertex> vertices{
		{ -1.0f, -1.0f, 0.0f, 0xff0000ff },
		{ 1.0f, -1.0f, 0.0f, 0xffff0000 },
		{ -1.0f, 1.0f, 0.0f, 0xff00ff00 },
		{ 1.0f, 1.0f, 0.0f, 0xffffff00 } };

	Collection::Vector<uint16_t> triangleIndices{ 0, 1, 2, 2, 1, 3 };

	new(destination) StaticMesh(std::move(vertices), std::move(triangleIndices));

	return true;
}

StaticMesh::StaticMesh(Collection::Vector<Vertex>&& vertices, Collection::Vector<uint16_t>&& triangleIndices)
	: m_vertices(vertices)
	, m_triangleIndices(triangleIndices)
{}
}
