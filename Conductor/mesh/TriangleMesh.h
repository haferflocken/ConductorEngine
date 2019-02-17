#pragma once

#include <mesh/Vertex.h>

#include <collection/Vector.h>
#include <file/Path.h>
#include <math/Matrix4x4.h>

namespace Mesh
{
class TriangleMesh
{
public:
	static constexpr const char* k_fileType = ".cms";
	static constexpr uint16_t k_invalidBoneParentIndex = UINT16_MAX;

	static const TriangleMesh k_simpleQuad;

	static bool TryLoad(const File::Path& filePath, TriangleMesh* destination);
	static void SaveToFile(const File::Path& filePath, const TriangleMesh& mesh);

	TriangleMesh(const Collection::Vector<PosColourVertex>& vertices, Collection::Vector<uint16_t>&& triangleIndices);

	TriangleMesh(const CompactVertexDeclaration& vertexDeclaration,
		Collection::Vector<uint8_t>&& vertexData,
		Collection::Vector<uint16_t>&& triangleIndices,
		Collection::Vector<Math::Matrix4x4>&& boneToParentTransforms,
		Collection::Vector<uint16_t>&& boneParentIndices);

	const CompactVertexDeclaration& GetVertexDeclaration() const { return m_vertexDeclaration; }
	const Collection::Vector<uint8_t>& GetVertexData() const { return m_vertexData; }
	const Collection::Vector<uint16_t>& GetTriangleIndices() const { return m_triangleIndices; }

	const Collection::Vector<Math::Matrix4x4>& GetBoneToParentTransforms() const { return m_boneToParentTransforms; }
	const Collection::Vector<uint16_t>& GetBoneParentIndices() const { return m_boneParentIndices; }

private:
	CompactVertexDeclaration m_vertexDeclaration;
	Collection::Vector<uint8_t> m_vertexData;
	Collection::Vector<uint16_t> m_triangleIndices;

	Collection::Vector<Math::Matrix4x4> m_boneToParentTransforms;
	Collection::Vector<uint16_t> m_boneParentIndices;
};
}
