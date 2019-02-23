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
		AMP_LOG_WARNING("Failed to initialize the FbxScene.");
		return nullptr;
	}

	ScopedPtr<FbxImporter> importer{ FbxImporter::Create(&fbxManager, "") };
	if (!importer->Initialize(filePath, -1, fbxManager.GetIOSettings()))
	{
		AMP_LOG_WARNING("Failed to initialize the FbxImporter.");
		return nullptr;
	}

	if (!importer->IsFBX())
	{
		AMP_LOG_WARNING("Failed to import [%s] because it isn't a valid FBX file.", filePath);
		return nullptr;
	}

	if (!importer->Import(fbxScene.Get()))
	{
		AMP_LOG_WARNING("Failed to import [%s].", filePath);
		return nullptr;
	}

	FbxSceneCheckUtility sceneCheck{ fbxScene.Get() };
	if (!sceneCheck.Validate())
	{
		AMP_LOG_WARNING("Failed to validate the FBX scene in [%s].", filePath);
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
		AMP_LOG_WARNING("Failed to initialize the FbxManager.");
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

	// Read in the vertex positions and UVs.
	Collection::Vector<uint8_t> vertexData;
	for (int nodeIndex = 0, numNodes = rootNode->GetChildCount(); nodeIndex < numNodes; ++nodeIndex)
	{
		FbxNode* const node = rootNode->GetChild(nodeIndex);
		const FbxNodeAttribute::EType attributeType = node->GetNodeAttribute()->GetAttributeType();
		if (attributeType != FbxNodeAttribute::eMesh)
		{
			continue;
		}
		FbxMesh& mesh = *static_cast<FbxMesh*>(node->GetNodeAttribute());

		const int elementUVCount = mesh.GetElementUVCount();
		if (elementUVCount <= 0)
		{
			AMP_LOG_WARNING("Imported FBX files must have UVs.");
			continue;
		}
		FbxGeometryElementUV& uvElement = *mesh.GetElementUV();
		if (uvElement.GetMappingMode() != FbxGeometryElement::eByControlPoint)
		{
			AMP_LOG_WARNING("Only one UV per vertex is supported.");
			continue;
		}
		const FbxLayerElement::EReferenceMode uvReferenceMode = uvElement.GetReferenceMode();
		if (uvReferenceMode != FbxGeometryElement::eDirect && uvReferenceMode != FbxGeometryElement::eIndexToDirect)
		{
			AMP_LOG_WARNING("Only direct or index to direct UV references are supported.");
			continue;
		}
		
		FbxVector4* const controlPoints = mesh.GetControlPoints();

		// Convert the control points into vertex data.
		for (int i = 0, numControlPoints = mesh.GetControlPointsCount(); i < numControlPoints; ++i)
		{
			// Add the vertex position data.
			FbxVector4& vertex = controlPoints[i];
			const std::array<float, 3> vertexCoords{
				static_cast<float>(vertex[0]),
				static_cast<float>(vertex[1]),
				static_cast<float>(vertex[2]) };

			vertexData.AddAll({
				reinterpret_cast<const uint8_t*>(vertexCoords.data()), vertexCoords.size() * sizeof(float) });

			// Add UV data.
			FbxVector2 fbxUV;
			if (uvReferenceMode == FbxGeometryElement::eDirect)
			{
				fbxUV = uvElement.GetDirectArray().GetAt(i);
			}
			else
			{
				const int id = uvElement.GetIndexArray().GetAt(i);
				fbxUV = uvElement.GetDirectArray().GetAt(id);
			}
			const std::array<float, 2> uvCoords{ static_cast<float>(fbxUV[0]), static_cast<float>(fbxUV[1]) };
			vertexData.AddAll({ reinterpret_cast<const uint8_t*>(uvCoords.data()), uvCoords.size() * sizeof(float) });

			// Add space for the weight group.
			vertexData.Add(UINT8_MAX);
			vertexData.Add(UINT8_MAX);
			vertexData.Add(UINT8_MAX);
			vertexData.Add(UINT8_MAX);
		}

		// Read in only one mesh.
		break;
	}

	// Read in the weight groups.
	Collection::Vector<float> weightGroups;
	for (int nodeIndex = 0, numNodes = rootNode->GetChildCount(); nodeIndex < numNodes; ++nodeIndex)
	{
		FbxNode* const node = rootNode->GetChild(nodeIndex);
		const FbxNodeAttribute::EType attributeType = node->GetNodeAttribute()->GetAttributeType();
		if (attributeType != FbxNodeAttribute::eMesh)
		{
			continue;
		}
		FbxMesh& mesh = *static_cast<FbxMesh*>(node->GetNodeAttribute());

		const int numSkins = mesh.GetDeformerCount(FbxDeformer::eSkin);
		for (int skinIndex = 0; skinIndex < numSkins; ++skinIndex)
		{
			FbxSkin* const skin = static_cast<FbxSkin*>(mesh.GetDeformer(skinIndex, FbxDeformer::eSkin));
			if (skin == nullptr)
			{
				continue;
			}

			// TODO 
		}

		// Read in only one mesh.
		break;
	}

	// Read in the triangles.
	Collection::Vector<uint16_t> triangleIndices;
	for (int nodeIndex = 0, numNodes = rootNode->GetChildCount(); nodeIndex < numNodes; ++nodeIndex)
	{
		FbxNode* const node = rootNode->GetChild(nodeIndex);
		const FbxNodeAttribute::EType attributeType = node->GetNodeAttribute()->GetAttributeType();
		if (attributeType != FbxNodeAttribute::eMesh)
		{
			continue;
		}
		FbxMesh& mesh = *static_cast<FbxMesh*>(node->GetNodeAttribute());

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

		// Read in only one mesh.
		break;
	}

	// Read the skeleton.
	Collection::Vector<Math::Matrix4x4> boneToParentTransforms;
	Collection::Vector<uint16_t> boneParentIndices;
	for (int nodeIndex = 0, numNodes = rootNode->GetChildCount(); nodeIndex < numNodes; ++nodeIndex)
	{
		FbxNode* const node = rootNode->GetChild(nodeIndex);
		const FbxNodeAttribute::EType attributeType = node->GetNodeAttribute()->GetAttributeType();
		if (attributeType != FbxNodeAttribute::eSkeleton)
		{
			continue;
		}

		const FbxSkeleton& skeletonRoot = *static_cast<FbxSkeleton*>(node->GetNodeAttribute());
		if (skeletonRoot.GetSkeletonType() != FbxSkeleton::eRoot)
		{
			continue;
		}

		struct StackElement final
		{
			size_t m_parentIndex;
			FbxNode* m_node;
		};
		Collection::Vector<StackElement> stack;
		stack.Add({ Mesh::TriangleMesh::k_invalidBoneIndex, node });

		while (!stack.IsEmpty())
		{
			const StackElement current = stack.Back();
			stack.RemoveLast();

			const FbxSkeleton& skeletonNode = *static_cast<const FbxSkeleton*>(current.m_node->GetNodeAttribute());
			const size_t boneIndex = boneToParentTransforms.Size();

			Math::Matrix4x4& boneToParentTransform = boneToParentTransforms.Emplace();
			const FbxAMatrix& localTransform = current.m_node->EvaluateLocalTransform();
			for (size_t columnIndex = 0; columnIndex < 4; ++columnIndex)
			{
				const FbxVector4& src = localTransform.GetColumn(static_cast<int>(columnIndex));
				Math::Vector4& dest = boneToParentTransform.GetColumn(columnIndex);
				dest.x = static_cast<float>(src[0]);
				dest.y = static_cast<float>(src[1]);
				dest.z = static_cast<float>(src[2]);
				dest.w = static_cast<float>(src[3]);
			}

			boneParentIndices.Add(static_cast<uint16_t>(current.m_parentIndex));

			const int numChildren = current.m_node->GetChildCount();
			for (int childIndex = 0; childIndex < numChildren; ++childIndex)
			{
				FbxNode* const childNode = current.m_node->GetChild(childIndex);
				const FbxNodeAttribute::EType childType = childNode->GetNodeAttribute()->GetAttributeType();
				if (childType == FbxNodeAttribute::eSkeleton)
				{
					stack.Add({ boneIndex, childNode });
				}
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
