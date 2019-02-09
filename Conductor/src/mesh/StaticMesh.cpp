#include <mesh/StaticMesh.h>

#include <file/FullFileReader.h>

#include <fstream>

namespace Mesh
{
namespace Internal_StaticMesh
{
static constexpr const char k_magic[] = "!!!!!!!!!CONDUCTOR MESH";
static constexpr const char k_channelSeparator[] = "!!!!";
static constexpr const uint32_t k_version = 2;

// The file header of mesh files. This must not contain any padding.
struct FileHeader final
{
	char m_magic[sizeof(k_magic)];
	uint32_t m_versionNumber;
	uint32_t m_numVertexAttributes;
	uint32_t m_numVertices;
	uint32_t m_numTriangleIndices;
};
static_assert(sizeof(FileHeader) == (sizeof(k_magic) + (sizeof(uint32_t) * 4)),
	"The mesh file header must not contain padding!");
}

const StaticMesh StaticMesh::k_simpleQuad{
	Collection::Vector<PosColourVertex>({
		{ -1.0f, -1.0f, 0.0f, 0xff0000ff },
		{ 1.0f, -1.0f, 0.0f, 0xffff0000 },
		{ -1.0f, 1.0f, 0.0f, 0xff00ff00 },
		{ 1.0f, 1.0f, 0.0f, 0xffffff00 }}),
	Collection::Vector<uint16_t>({ 0, 1, 2, 2, 1, 3 }) };

bool StaticMesh::TryLoad(const File::Path& filePath, StaticMesh* destination)
{
	using namespace Internal_StaticMesh;

	const std::string rawFile = File::ReadFullTextFile(filePath);
	const size_t rawFileLengthInBytes = rawFile.length();

	if (rawFileLengthInBytes < sizeof(FileHeader))
	{
		return false;
	}

	FileHeader header;
	memcpy(&header, rawFile.data(), sizeof(FileHeader));

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
	
	// Read the vertex attributes.
	Collection::Vector<VertexAttribute> attributes;
	const char* fileIter = rawFile.data() + sizeof(FileHeader);
	const char* const fileEnd = rawFile.data() + rawFileLengthInBytes;
	for (size_t i = 0; i < header.m_numVertexAttributes; ++i)
	{
		const VertexAttribute attribute = ConvertStringToVertexAttribute(fileIter);
		if (attribute == VertexAttribute::Invalid)
		{
			return false;
		}
		// Attributes are expected in the order they are declared in VertexAttribute.
		if ((!attributes.IsEmpty()) && static_cast<uint32_t>(attribute) <= static_cast<uint32_t>(attributes.Back()))
		{
			return false;
		}

		attributes.Add(attribute);

		while (*fileIter != '\0' && fileIter < fileEnd)
		{
			++fileIter;
		}
		if (fileIter == fileEnd)
		{
			return false;
		}
		++fileIter;
	}
	const CompactVertexDeclaration vertexDeclaration{ attributes.GetConstView() };
	const ExpandedVertexDeclaration expandedVertexDeclaration = vertexDeclaration.Expand();

	// Validate the file's size.
	const char* const fileVertexDataBegin = fileIter;
	const int64_t numBytesLeftInFile = fileEnd - fileVertexDataBegin;

	const uint32_t expectedSizeOfVertexData =
		(header.m_numVertices * vertexDeclaration.GetVertexSizeInBytes())
		+ (header.m_numVertexAttributes * sizeof(k_channelSeparator));
	const uint32_t expectedSizeOfIndexData = (header.m_numTriangleIndices * sizeof(uint16_t));

	const int64_t expectedSizeOfVertexAndIndexData = expectedSizeOfVertexData + expectedSizeOfIndexData;
	if (numBytesLeftInFile != expectedSizeOfVertexAndIndexData)
	{
		return false;
	}

	// Read the vertex data. This is a transformation from a struct-of-arrays to an array-of-structs.
	Collection::Vector<uint8_t> vertexData;
	vertexData.Resize(expectedSizeOfVertexData);

	const char* fileVertexChannelIter = fileVertexDataBegin;
	for (size_t attributeIndex = 0; attributeIndex  < header.m_numVertexAttributes; ++attributeIndex)
	{
		const uint32_t attributeSize = expandedVertexDeclaration.m_attributeSizesInBytes[attributeIndex];
		const uint32_t attributeOffset = expandedVertexDeclaration.m_attributeOffsets[attributeIndex];
		const size_t channelSize = attributeSize * header.m_numVertices;

		for (size_t vertexIndex = 0; vertexIndex < header.m_numVertices; ++vertexIndex)
		{
			const char* const src = &fileVertexChannelIter[vertexIndex * attributeSize];
			uint8_t* const vertex = &vertexData[vertexIndex * vertexDeclaration.GetVertexSizeInBytes()];
			uint8_t* const dest = vertex + attributeOffset;
			memcpy(dest, src, attributeSize);
		}

		fileVertexChannelIter += channelSize;
		fileVertexChannelIter += sizeof(k_channelSeparator);
	}

	// Read in the index data.
	const char* const rawIndices = fileVertexDataBegin + expectedSizeOfVertexData;
	AMP_FATAL_ASSERT(rawIndices == fileVertexChannelIter, "The indices should be directly after the vertices.");

	const Collection::ArrayView<const uint16_t> indices{
		reinterpret_cast<const uint16_t*>(rawIndices), header.m_numTriangleIndices };

	// Create the mesh.
	new(destination) StaticMesh(vertexDeclaration, std::move(vertexData), Collection::Vector<uint16_t>(indices));

	return true;
}

void StaticMesh::SaveToFile(const File::Path& filePath, const StaticMesh& mesh)
{
	using namespace Internal_StaticMesh;

	const ExpandedVertexDeclaration expandedVertexDeclaration = mesh.GetVertexDeclaration().Expand();

	// Write the FileHeader.
	std::ofstream output{ filePath.c_str(), std::ios_base::binary | std::ios_base::out | std::ios_base::trunc };

	FileHeader header;
	memcpy(header.m_magic, k_magic, sizeof(k_magic));
	header.m_versionNumber = k_version;
	header.m_numVertexAttributes = expandedVertexDeclaration.m_numAttributes;
	header.m_numVertices = mesh.GetVertexData().Size() / expandedVertexDeclaration.m_vertexSizeInBytes;
	header.m_numTriangleIndices = mesh.GetTriangleIndices().Size();

	const char* const rawHeader = reinterpret_cast<const char*>(&header);
	output.write(rawHeader, sizeof(FileHeader));

	// Write the vertex declaration.
	for (size_t attributeIndex = 0; attributeIndex < expandedVertexDeclaration.m_numAttributes; ++attributeIndex)
	{
		const VertexAttribute attribute = expandedVertexDeclaration.m_attributes[attributeIndex];
		const char* const attributeString = GetAttributeString(attribute);
		const size_t attributeStringLength = strlen(attributeString);
		// Write the attribute string with its null terminator.
		output.write(attributeString, attributeStringLength + 1);
	}

	// Write the vertices. This is a transformation from an array-of-structs to a struct-of-arrays.
	for (size_t attributeIndex = 0; attributeIndex < expandedVertexDeclaration.m_numAttributes; ++attributeIndex)
	{
		const uint32_t attributeSize = expandedVertexDeclaration.m_attributeSizesInBytes[attributeIndex];
		const uint32_t attributeOffset = expandedVertexDeclaration.m_attributeOffsets[attributeIndex];

		const char* const vertexData = reinterpret_cast<const char*>(&mesh.GetVertexData().Front());
		for (size_t vertexIndex = 0; vertexIndex < header.m_numVertices; ++vertexIndex)
		{
			const char* const vertex = &vertexData[vertexIndex * expandedVertexDeclaration.m_vertexSizeInBytes];
			const char* const attributeInVertex = vertex + attributeOffset;
			output.write(attributeInVertex, attributeSize);
		}
		output.write(k_channelSeparator, sizeof(k_channelSeparator));
	}

	// Write the triangle indices and flush the output.
	const char* const rawTriangleIndices = reinterpret_cast<const char*>(&mesh.GetTriangleIndices().Front());
	output.write(rawTriangleIndices, header.m_numTriangleIndices * sizeof(uint16_t));

	output.flush();
}

StaticMesh::StaticMesh(const Collection::Vector<PosColourVertex>& vertices,
	Collection::Vector<uint16_t>&& triangleIndices)
	: m_vertexDeclaration({ VertexAttribute::Position, VertexAttribute::Colour0 })
	, m_vertexData()
	, m_triangleIndices(std::move(triangleIndices))
{
	const uint32_t vertexSize = m_vertexDeclaration.GetVertexSizeInBytes();
	m_vertexData.Resize(vertices.Size() * vertexSize);
	memcpy(m_vertexData.begin(), vertices.begin(), m_vertexData.Size());
}

StaticMesh::StaticMesh(const CompactVertexDeclaration& vertexDeclaration,
	Collection::Vector<uint8_t>&& vertexData,
	Collection::Vector<uint16_t>&& triangleIndices)
	: m_vertexDeclaration(vertexDeclaration)
	, m_vertexData(std::move(vertexData))
	, m_triangleIndices(std::move(triangleIndices))
{
}
}
