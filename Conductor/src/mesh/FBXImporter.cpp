#include <mesh/FBXImporter.h>

#include <collection/Vector.h>
#include <mesh/TriangleMesh.h>

#include <fbxsdk.h>

namespace Internal_FBXImporter
{
template <typename T>
class ScopedPtr
{
	T* m_value;

public:
	explicit ScopedPtr(T* value)
		: m_value(value)
	{}

	~ScopedPtr()
	{
		if (m_value != nullptr)
		{
			m_value->Destroy();
			m_value = nullptr;
		}
	}

	bool IsValid() const { return m_value != nullptr; }

	T* Get() { return m_value; }
	const T* Get() const { return m_value; }

	T* Release()
	{
		T* const result = m_value;
		m_value = nullptr;
		return result;
	}

	T* operator->() { return m_value; }
	const T* operator->() const { return m_value; }

	T& operator*() { return *m_value; }
	const T& operator*() const { return *m_value; }
};

FbxScene* TryLoadScene(FbxManager& fbxManager, const char* const filePath)
{
	ScopedPtr<FbxScene> fbxScene{ FbxScene::Create(&fbxManager, "FBXScene") };
	if (!fbxScene.IsValid())
	{
		AMP_LOG_ERROR("Failed to initialize the FbxScene.");
		return nullptr;
	}

	ScopedPtr<FbxImporter> importer{ FbxImporter::Create(&fbxManager, "") };
	if (!importer->Initialize(filePath, -1, fbxManager.GetIOSettings()))
	{
		AMP_LOG_ERROR("Failed to initialize the FbxImporter.");
		return nullptr;
	}

	if (!importer->IsFBX())
	{
		AMP_LOG_ERROR("Failed to import [%s] because it isn't a valid FBX file.", filePath);
		return nullptr;
	}

	if (!importer->Import(fbxScene.Get()))
	{
		AMP_LOG_ERROR("Failed to import [%s].", filePath);
		return nullptr;
	}

	FbxSceneCheckUtility sceneCheck{ fbxScene.Get() };
	if (!sceneCheck.Validate())
	{
		AMP_LOG_ERROR("Failed to validate the FBX scene in [%s].", filePath);
		return nullptr;
	}

	return fbxScene.Release();
}
}

bool Mesh::TryImportFBX(const File::Path& filePath, TriangleMesh* destination)
{
	using namespace Internal_FBXImporter;

	ScopedPtr<FbxManager> fbxManager{ FbxManager::Create() };
	if (!fbxManager.IsValid())
	{
		AMP_LOG_ERROR("Failed to initialize the FbxManager.");
		return false;
	}

	ScopedPtr<FbxIOSettings> ioSettings{ FbxIOSettings::Create(fbxManager.Get(), "FBXRoot") };
	fbxManager->SetIOSettings(ioSettings.Get());

	ScopedPtr<FbxScene> fbxScene{ TryLoadScene(*fbxManager, filePath.string().c_str()) };
	if (!fbxScene.IsValid())
	{
		return false;
	}

	FbxNode* const rootNode = fbxScene->GetRootNode();
	if (rootNode == nullptr)
	{
		return false;
	}

	const Mesh::CompactVertexDeclaration vertexDeclaration{
		Mesh::VertexAttribute::Position, Mesh::VertexAttribute::TextureCoords0, Mesh::VertexAttribute::WeightGroup };

	Collection::Vector<uint8_t> vertexData;
	Collection::Vector<uint16_t> triangleIndices;
	Collection::Vector<Math::Matrix4x4> boneToParentTransforms;
	Collection::Vector<uint16_t> boneParentIndices;
	Collection::Vector<float> weightGroups;

	for (int nodeIndex = 0, numNodes = rootNode->GetChildCount(); nodeIndex < numNodes; ++nodeIndex)
	{
		FbxNode* const node = rootNode->GetChild(nodeIndex);
		const FbxNodeAttribute::EType attributeType = node->GetNodeAttribute()->GetAttributeType();
		switch (attributeType)
		{
		case FbxNodeAttribute::eMesh:
		{
			FbxMesh& mesh = *static_cast<FbxMesh*>(node->GetNodeAttribute());
			FbxVector4* const controlPoints = mesh.GetControlPoints();

			// Convert the control points into vertex data.
			for (int i = 0, numControlPoints = mesh.GetControlPointsCount(); i < numControlPoints; ++i)
			{
				// Add the vertex position data.
				FbxVector4& vertex = controlPoints[i];
				std::array<float, 3> vertexCoords{
					static_cast<float>(vertex[0]),
					static_cast<float>(vertex[1]),
					static_cast<float>(vertex[2]) };

				vertexData.AddAll({
					reinterpret_cast<const uint8_t*>(vertexCoords.data()), vertexCoords.size() * sizeof(float) });

				// TODO Add UV data.

				// Add space for the weight group.
				vertexData.Add(UINT8_MAX);
			}

			// Convert the polygons into triangles.
			const int numPolygons = mesh.GetPolygonCount();
			for (int i = 0; i < numPolygons; ++i)
			{
				const int numPolygonVertices = mesh.GetPolygonSize(i);
				for (int j = 2; j < numPolygonVertices; ++j)
				{
					triangleIndices.Add(mesh.GetPolygonVertex(i, j - 2));
					triangleIndices.Add(mesh.GetPolygonVertex(i, j - 1));
					triangleIndices.Add(mesh.GetPolygonVertex(i, j));
				}
			}
			break;
		}
		case FbxNodeAttribute::eSkeleton:
		{
			// TODO read the skeleton
			break;
		}
		}
	}

	destination = new (destination) Mesh::TriangleMesh(
		vertexDeclaration,
		std::move(vertexData),
		std::move(triangleIndices),
		std::move(boneToParentTransforms),
		std::move(boneParentIndices),
		std::move(weightGroups));
	return true;
}
