#include <mesh/TriangleMesh.h>

#include <file/FullFileReader.h>

#include <fstream>

namespace Mesh
{
namespace Internal_TriangleMesh
{
static constexpr const char k_magic[] = "!!!!!!!!!CONDUCTOR MESH";
static constexpr const char k_channelSeparator[] = "!!!!";
static constexpr const uint32_t k_version = 6;

// The file header of mesh files. This must not contain any padding.
struct FileHeader final
{
	char m_magic[sizeof(k_magic)];
	uint32_t m_versionNumber;
	uint32_t m_numVertexAttributes;
	uint32_t m_numVertices;
	uint32_t m_numTriangleIndices;
	uint32_t m_numBones;
	uint32_t m_numBoneNameBytes;
};
static_assert(sizeof(FileHeader) == (sizeof(k_magic) + (sizeof(uint32_t) * 6)),
	"The mesh file header must not contain padding!");
}

const TriangleMesh TriangleMesh::k_simpleQuad{
	Collection::Vector<PosColourVertex>({
		{ -1.0f, -1.0f, 0.0f, 0xff0000ff },
		{ 1.0f, -1.0f, 0.0f, 0xffff0000 },
		{ -1.0f, 1.0f, 0.0f, 0xff00ff00 },
		{ 1.0f, 1.0f, 0.0f, 0xffffff00 }}),
	Collection::Vector<uint16_t>({ 0, 1, 2, 2, 1, 3 }) };

bool TriangleMesh::TryLoad(const File::Path& filePath, TriangleMesh* destination)
{
	using namespace Internal_TriangleMesh;

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
	const uint32_t expectedSizeOfTriangleIndexData = (header.m_numTriangleIndices * sizeof(uint16_t));
	const uint32_t expectedSizeOfBoneTransforms = (header.m_numBones * sizeof(Math::Matrix4x4));
	const uint32_t expectedSizeOfBoneParentIndices = (header.m_numBones * sizeof(uint16_t));

	const int64_t expectedSizeOfPostHeaderData = expectedSizeOfVertexData + expectedSizeOfTriangleIndexData +
		expectedSizeOfBoneTransforms + expectedSizeOfBoneParentIndices + header.m_numBoneNameBytes;
	if (numBytesLeftInFile != expectedSizeOfPostHeaderData)
	{
		return false;
	}

	// Read the vertex data. This is a transformation from a struct-of-arrays to an array-of-structs.
	Collection::Vector<uint8_t> vertexData;
	vertexData.Resize(expectedSizeOfVertexData);

	const char* fileVertexChannelIter = fileVertexDataBegin;
	for (size_t attributeIndex = 0; attributeIndex < header.m_numVertexAttributes; ++attributeIndex)
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

	// Read in the triangle index data.
	const char* const rawTriangleIndices = fileVertexDataBegin + expectedSizeOfVertexData;
	AMP_FATAL_ASSERT(rawTriangleIndices == fileVertexChannelIter,
		"The triangle indices should be directly after the vertices.");

	Collection::Vector<uint16_t> triangleIndices;
	triangleIndices.Resize(header.m_numTriangleIndices);
	memcpy(triangleIndices.begin(), rawTriangleIndices, expectedSizeOfTriangleIndexData);

	// Read in data that is related to bones.
	Collection::Vector<Math::Matrix4x4> boneTransforms;
	Collection::Vector<uint16_t> boneParentIndices;
	Collection::Vector<std::string> boneNames;

	if (header.m_numBones > 0)
	{
		// Read in the bone data.
		const char* const rawBoneTransforms = rawTriangleIndices + expectedSizeOfTriangleIndexData;
		const char* const rawBoneParentIndices = rawBoneTransforms + expectedSizeOfBoneTransforms;
		const char* const rawBoneNames = rawBoneParentIndices + expectedSizeOfBoneParentIndices;

		boneTransforms.Resize(header.m_numBones);
		memcpy(boneTransforms.begin(), rawBoneTransforms, expectedSizeOfBoneTransforms);

		boneParentIndices.Resize(header.m_numBones);
		memcpy(boneParentIndices.begin(), rawBoneParentIndices, expectedSizeOfBoneParentIndices);

		boneNames.Resize(header.m_numBones);
		const char* boneNameInputIter = rawBoneNames;
		for (auto& boneName : boneNames)
		{
			for (; *boneNameInputIter != '\0'; ++boneNameInputIter)
			{
				const char c = *boneNameInputIter;
				boneName.push_back(c);
			}
			// Advance past the null terminator.
			++boneNameInputIter;
		}
	}

	// Create the mesh.
	destination = new(destination) TriangleMesh(
		vertexDeclaration,
		std::move(vertexData),
		std::move(triangleIndices),
		std::move(boneTransforms),
		std::move(boneParentIndices),
		std::move(boneNames));

	return true;
}

void TriangleMesh::SaveToFile(const File::Path& filePath, const TriangleMesh& mesh)
{
	using namespace Internal_TriangleMesh;

	const ExpandedVertexDeclaration expandedVertexDeclaration = mesh.GetVertexDeclaration().Expand();

	// Write the FileHeader.
	std::ofstream output{ filePath.c_str(), std::ios_base::binary | std::ios_base::out | std::ios_base::trunc };

	FileHeader header;
	memcpy(header.m_magic, k_magic, sizeof(k_magic));
	header.m_versionNumber = k_version;
	header.m_numVertexAttributes = expandedVertexDeclaration.m_numAttributes;
	header.m_numVertices = mesh.GetVertexData().Size() / expandedVertexDeclaration.m_vertexSizeInBytes;
	header.m_numTriangleIndices = mesh.GetTriangleIndices().Size();
	header.m_numBones = mesh.GetBoneToParentTransforms().Size();

	header.m_numBoneNameBytes = 0;
	for (const auto& boneName : mesh.GetBoneNames())
	{
		header.m_numBoneNameBytes += static_cast<uint32_t>(boneName.length());
		header.m_numBoneNameBytes += 1; // Add one for the null terminator.
	}

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

	// Write the triangle indices.
	const char* const rawTriangleIndices = reinterpret_cast<const char*>(&mesh.GetTriangleIndices().Front());
	output.write(rawTriangleIndices, header.m_numTriangleIndices * sizeof(uint16_t));

	// Write the bone data.
	const char* const rawBoneTransforms = reinterpret_cast<const char*>(&mesh.GetBoneToParentTransforms().Front());
	output.write(rawBoneTransforms, header.m_numBones * sizeof(Math::Matrix4x4));

	const char* const rawBoneParentIndices = reinterpret_cast<const char*>(&mesh.GetBoneParentIndices().Front());
	output.write(rawBoneParentIndices, header.m_numBones * sizeof(uint16_t));

	// Write the bone names, separated by null terminators.
	for (const auto& boneName : mesh.GetBoneNames())
	{
		output.write(boneName.c_str(), boneName.length());
		output.put('\0');
	}

	// Flush the output.
	output.flush();
}

TriangleMesh::TriangleMesh(const Collection::Vector<PosColourVertex>& vertices,
	Collection::Vector<uint16_t>&& triangleIndices)
	: m_vertexDeclaration({ VertexAttribute::Position, VertexAttribute::Colour0 })
	, m_vertexData()
	, m_triangleIndices(std::move(triangleIndices))
{
	const uint32_t vertexSize = m_vertexDeclaration.GetVertexSizeInBytes();
	m_vertexData.Resize(vertices.Size() * vertexSize);
	memcpy(m_vertexData.begin(), vertices.begin(), m_vertexData.Size());
}

TriangleMesh::TriangleMesh(const CompactVertexDeclaration& vertexDeclaration,
	Collection::Vector<uint8_t>&& vertexData,
	Collection::Vector<uint16_t>&& triangleIndices,
	Collection::Vector<Math::Matrix4x4>&& boneToParentTransforms,
	Collection::Vector<uint16_t>&& boneParentIndices,
	Collection::Vector<std::string>&& boneNames)
	: m_vertexDeclaration(vertexDeclaration)
	, m_vertexData(std::move(vertexData))
	, m_triangleIndices(std::move(triangleIndices))
	, m_boneToParentTransforms(std::move(boneToParentTransforms))
	, m_boneParentIndices(std::move(boneParentIndices))
	, m_boneNames(std::move(boneNames))
{
}
}
