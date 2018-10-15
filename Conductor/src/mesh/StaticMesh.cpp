#include <mesh/StaticMesh.h>

#include <file/FullFileReader.h>

#include <fstream>

namespace Mesh
{
namespace Internal_StaticMesh
{
static constexpr const char k_magic[] = "!!!!!CONDUCTOR STATIC MESH!!!!!";
static constexpr const uint32_t k_version = 1;

// The file header of StaticMesh files. This must not contain any padding.
struct FileHeader final
{
	char m_magic[sizeof(k_magic)];
	uint32_t m_versionNumber;
	uint32_t m_numVertices;
	uint32_t m_numTriangleIndices;
};
}

const StaticMesh StaticMesh::k_simpleQuad{
	Collection::Vector<Vertex>({
		{ -1.0f, -1.0f, 0.0f, 0xff0000ff },
		{ 1.0f, -1.0f, 0.0f, 0xffff0000 },
		{ -1.0f, 1.0f, 0.0f, 0xff00ff00 },
		{ 1.0f, 1.0f, 0.0f, 0xffffff00 }}),
	Collection::Vector<uint16_t>({ 0, 1, 2, 2, 1, 3 })};

bool StaticMesh::TryLoad(const File::Path& filePath, StaticMesh* destination)
{
	using namespace Internal_StaticMesh;

	const std::string rawFile = File::ReadFullTextFile(filePath);
	const size_t rawFileLengthInBytes = rawFile.length();

	if (rawFileLengthInBytes < sizeof(FileHeader))
	{
		return false;
	}

	const FileHeader& header = *reinterpret_cast<const FileHeader*>(rawFile.data());

	// Validate that the file starts with the magic constant.
	if (memcmp(header.m_magic, k_magic, sizeof(k_magic)) != 0)
	{
		return false;
	}

	// Validate the file's version.
	if (header.m_versionNumber != k_version)
	{
		return false;
	}

	// Validate the file's size.
	const size_t expectedSizeInBytes = sizeof(FileHeader)
		+ (header.m_numVertices * sizeof(Vertex))
		+ (header.m_numTriangleIndices * sizeof(uint16_t));
	if (rawFileLengthInBytes != expectedSizeInBytes)
	{
		return false;
	}

	// Read in the data.
	const char* const rawVertices = rawFile.data() + sizeof(FileHeader);
	const char* const rawIndices = rawVertices + (header.m_numVertices * sizeof(Vertex));

	const Collection::ArrayView<const Vertex> vertices{
		reinterpret_cast<const Vertex*>(rawVertices), header.m_numVertices };
	const Collection::ArrayView<const uint16_t> indices{
		reinterpret_cast<const uint16_t*>(rawIndices), header.m_numTriangleIndices };

	new(destination) StaticMesh(Collection::Vector<Vertex>(vertices), Collection::Vector<uint16_t>(indices));

	return true;
}

void StaticMesh::SaveToFile(const File::Path& filePath, const StaticMesh& mesh)
{
	using namespace Internal_StaticMesh;

	std::ofstream output{ filePath.c_str(), std::ios_base::binary | std::ios_base::out | std::ios_base::trunc };

	FileHeader header;
	memcpy(header.m_magic, k_magic, sizeof(k_magic));
	header.m_versionNumber = k_version;
	header.m_numVertices = mesh.GetVertices().Size();
	header.m_numTriangleIndices = mesh.GetTriangleIndices().Size();

	const char* const rawHeader = reinterpret_cast<const char*>(&header);
	const char* const rawVertices = reinterpret_cast<const char*>(&mesh.GetVertices().Front());
	const char* const rawTriangleIndices = reinterpret_cast<const char*>(&mesh.GetTriangleIndices().Front());

	output.write(rawHeader, sizeof(FileHeader));
	output.write(rawVertices, header.m_numVertices * sizeof(Vertex));
	output.write(rawTriangleIndices, header.m_numTriangleIndices * sizeof(uint16_t));

	output.flush();
}

StaticMesh::StaticMesh(Collection::Vector<Vertex>&& vertices, Collection::Vector<uint16_t>&& triangleIndices)
	: m_vertices(vertices)
	, m_triangleIndices(triangleIndices)
{}
}
