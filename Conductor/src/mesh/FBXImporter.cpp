#include <mesh/FBXImporter.h>

#include <collection/Vector.h>
#include <collection/VectorMap.h>
#include <math/MathConstants.h>
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

void ReadVertexPositions(
	const FbxMesh& mesh,
	const Mesh::ExpandedVertexDeclaration& expandedVertexDeclaration,
	Collection::Vector<uint8_t>& outVertexData)
{
	// Convert the control points into vertex data.
	const size_t sizeOfVertex = expandedVertexDeclaration.m_vertexSizeInBytes;
	const size_t vertexPositionOffset = expandedVertexDeclaration.m_attributeOffsets[0];

	const FbxVector4* const controlPoints = mesh.GetControlPoints();
	const int numControlPoints = mesh.GetControlPointsCount();

	for (int i = 0; i < numControlPoints; ++i)
	{
		// Get the vertex position data.
		const FbxVector4& controlPoint = controlPoints[i];
		const std::array<float, 3> vertexCoords{
			static_cast<float>(controlPoint[0]),
			static_cast<float>(controlPoint[1]),
			static_cast<float>(controlPoint[2]) };

		// Copy the data into the vertex.
		uint8_t* const vertex = &outVertexData[i * sizeOfVertex];
		memcpy(vertex + vertexPositionOffset, vertexCoords.data(), vertexCoords.size() * sizeof(float));
	}
}

bool TryReadVertexTextureCoordinates(
	FbxMesh& mesh,
	const Mesh::ExpandedVertexDeclaration& expandedVertexDeclaration,
	Collection::Vector<uint8_t>& outVertexData)
{
	const int elementUVCount = mesh.GetElementUVCount();
	if (elementUVCount <= 0)
	{
		AMP_LOG_WARNING("Imported FBX files must have UVs.");
		return false;
	}
	FbxGeometryElementUV& uvElement = *mesh.GetElementUV();
	const FbxLayerElement::EMappingMode uvMappingMode = uvElement.GetMappingMode();
	if (uvMappingMode != FbxGeometryElement::eByControlPoint && uvMappingMode != FbxGeometryElement::eByPolygonVertex)
	{
		AMP_LOG_WARNING("Unsupported UV mapping mode [%d].", static_cast<int32_t>(uvMappingMode));
		return false;
	}
	const FbxLayerElement::EReferenceMode uvReferenceMode = uvElement.GetReferenceMode();
	if (uvReferenceMode != FbxGeometryElement::eDirect && uvReferenceMode != FbxGeometryElement::eIndexToDirect)
	{
		AMP_LOG_WARNING("Only direct or index to direct UV references are supported.");
		return false;
	}

	const size_t sizeOfVertex = expandedVertexDeclaration.m_vertexSizeInBytes;
	const size_t vertexTextureCoordsOffset = expandedVertexDeclaration.m_attributeOffsets[1];

	const FbxVector4* const controlPoints = mesh.GetControlPoints();
	const int numControlPoints = mesh.GetControlPointsCount();

	if (uvMappingMode == FbxGeometryElement::eByControlPoint)
	{
		for (int i = 0, numControlPoints = mesh.GetControlPointsCount(); i < numControlPoints; ++i)
		{
			// Get UV data.
			FbxVector2 fbxUV;
			if (uvReferenceMode == FbxGeometryElement::eDirect)
			{
				fbxUV = uvElement.GetDirectArray().GetAt(i);
			}
			else
			{
				const int uvIndex = uvElement.GetIndexArray().GetAt(i);
				fbxUV = uvElement.GetDirectArray().GetAt(uvIndex);
			}
			const std::array<float, 2> uvCoords{ static_cast<float>(fbxUV[0]), static_cast<float>(fbxUV[1]) };

			// Copy the data into the vertex.
			uint8_t* const vertex = &outVertexData[i * sizeOfVertex];
			memcpy(vertex + vertexTextureCoordsOffset, uvCoords.data(), uvCoords.size() * sizeof(float));
		}
	}
	else
	{
		const int numPolygons = mesh.GetPolygonCount();
		for (int i = 0; i < numPolygons; ++i)
		{
			const int numPolygonVertices = mesh.GetPolygonSize(i);
			for (int j = 0; j < numPolygonVertices; ++j)
			{
				const int uvIndex = mesh.GetTextureUVIndex(i, j);
				const FbxVector2 fbxUV = uvElement.GetDirectArray().GetAt(uvIndex);
				const std::array<float, 2> uvCoords{ static_cast<float>(fbxUV[0]), static_cast<float>(fbxUV[1]) };

				// Copy the data into the vertex.
				const int controlPointIndex = mesh.GetPolygonVertex(i, j);
				uint8_t* const vertex = &outVertexData[controlPointIndex * sizeOfVertex];
				memcpy(vertex + vertexTextureCoordsOffset, uvCoords.data(), uvCoords.size() * sizeof(float));
			}
		}
	}

	return true;
}

Collection::VectorMap<const FbxNode*, const FbxCluster*> GetBoneNodesToClusters(FbxMesh& mesh)
{
	Collection::VectorMap<const FbxNode*, const FbxCluster*> boneNodesToClusters;

	const int numSkins = mesh.GetDeformerCount(FbxDeformer::eSkin);
	for (int skinIndex = 0; skinIndex < numSkins; ++skinIndex)
	{
		FbxSkin* const skin = static_cast<FbxSkin*>(mesh.GetDeformer(skinIndex, FbxDeformer::eSkin));
		if (skin == nullptr)
		{
			continue;
		}

		// We expect a cluster per bone.
		const int numClusters = skin->GetClusterCount();
		for (int i = 0; i < numClusters; ++i)
		{
			const FbxCluster* const cluster = skin->GetCluster(i);
			const FbxNode* const linkedBoneNode = cluster->GetLink();
			if (linkedBoneNode != nullptr)
			{
				boneNodesToClusters[linkedBoneNode] = cluster;
			}
		}

		// Only process a single skin.
		break;
	}

	return boneNodesToClusters;
}

void ConvertFBXMatrixToConductorMatrix(const FbxAMatrix& src, Math::Matrix4x4& dest)
{
	for (int columnIndex = 0; columnIndex < 4; ++columnIndex)
	{
		Math::Vector4& destColumn = dest.GetColumn(columnIndex);
		destColumn.x = static_cast<float>(src.Get(columnIndex, 0));
		destColumn.y = static_cast<float>(src.Get(columnIndex, 1));
		destColumn.z = static_cast<float>(src.Get(columnIndex, 2));
		destColumn.w = static_cast<float>(src.Get(columnIndex, 3));
	}
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

	// Find the mesh.
	FbxMesh* mesh = nullptr;
	for (int nodeIndex = 0, numNodes = rootNode->GetChildCount(); nodeIndex < numNodes; ++nodeIndex)
	{
		FbxNode* const node = rootNode->GetChild(nodeIndex);
		const FbxNodeAttribute::EType attributeType = node->GetNodeAttribute()->GetAttributeType();
		if (attributeType == FbxNodeAttribute::eMesh)
		{
			mesh = static_cast<FbxMesh*>(node->GetNodeAttribute());
			break;
		}
	}
	if (mesh == nullptr)
	{
		return false;
	}

	// Create the vertex declaration.
	CompactVertexDeclaration vertexDeclaration;

	const int numSkins = mesh->GetDeformerCount(FbxDeformer::eSkin);
	if (numSkins == 0)
	{
		vertexDeclaration = CompactVertexDeclaration{ VertexAttribute::Position, VertexAttribute::TextureCoords0 };
	}
	else
	{
		vertexDeclaration = CompactVertexDeclaration{
			VertexAttribute::Position, VertexAttribute::TextureCoords0, VertexAttribute::BoneWeights };
	}

	const ExpandedVertexDeclaration expandedVertexDeclaration = vertexDeclaration.Expand();

	// Initialize vertexData.
	const int numControlPoints = mesh->GetControlPointsCount();
	const uint32_t sizeOfVertex = vertexDeclaration.GetVertexSizeInBytes();
	const uint32_t sizeOfVertexData = sizeOfVertex * numControlPoints;

	Collection::Vector<uint8_t> vertexData;
	vertexData.Resize(sizeOfVertexData, UINT8_MAX);

	// Read in the vertex positions and texture coordinates.
	ReadVertexPositions(*mesh, expandedVertexDeclaration, vertexData);
	if (!TryReadVertexTextureCoordinates(*mesh, expandedVertexDeclaration, vertexData))
	{
		return false;
	}

	// Convert the polygons into triangles.
	Collection::Vector<uint16_t> triangleIndices;
	const int numPolygons = mesh->GetPolygonCount();
	for (int i = 0; i < numPolygons; ++i)
	{
		const int numPolygonVertices = mesh->GetPolygonSize(i);
		for (int j = 0; j < numPolygonVertices; j += 2)
		{
			const int j1 = (j + 1) % numPolygonVertices;
			const int j2 = (j + 2) % numPolygonVertices;

			triangleIndices.Add(mesh->GetPolygonVertex(i, j2));
			triangleIndices.Add(mesh->GetPolygonVertex(i, j1));
			triangleIndices.Add(mesh->GetPolygonVertex(i, j));
		}
	}

	// Read the skeleton and bone weights.
	const Collection::VectorMap<const FbxNode*, const FbxCluster*> boneNodesToClusters = GetBoneNodesToClusters(*mesh);
	const uint32_t vertexBoneWeightsOffset =
		(numSkins == 0) ? UINT32_MAX : expandedVertexDeclaration.m_attributeOffsets[2];

	Collection::Vector<Math::Matrix4x4> boneInverseGlobalTransforms;
	Collection::Vector<Math::Matrix4x4> boneToParentTransforms;
	Collection::Vector<uint16_t> boneParentIndices;
	Collection::Vector<std::string> boneNames;
	for (int nodeIndex = 0, numNodes = rootNode->GetChildCount(); nodeIndex < numNodes; ++nodeIndex)
	{
		FbxNode* const node = rootNode->GetChild(nodeIndex);
		const FbxNodeAttribute::EType attributeType = node->GetNodeAttribute()->GetAttributeType();

		Collection::Vector<FbxNode*> skeletonRootNodes;
		if (attributeType == FbxNodeAttribute::eSkeleton)
		{
			const FbxSkeleton& skeletonRoot = *static_cast<FbxSkeleton*>(node->GetNodeAttribute());
			if (skeletonRoot.GetSkeletonType() == FbxSkeleton::eRoot)
			{
				skeletonRootNodes.Add(node);
			}
		}
		else if (attributeType == FbxNodeAttribute::eNull)
		{
			// Collect all skeleton nodes within this null node.
			const int numChildren = node->GetChildCount();
			for (int childIndex = 0; childIndex < numChildren; ++childIndex)
			{
				FbxNode* const childNode = node->GetChild(childIndex);
				if (childNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
				{
					skeletonRootNodes.Add(childNode);
				}
			}
		}

		if (skeletonRootNodes.IsEmpty())
		{
			continue;
		}

		struct StackElement final
		{
			size_t m_parentIndex;
			FbxNode* m_node;
		};
		Collection::Vector<StackElement> stack;
		for (auto& skeletonRootNode : skeletonRootNodes)
		{
			stack.Add({ Mesh::TriangleMesh::k_invalidBoneIndex, skeletonRootNode });
		}

		while (!stack.IsEmpty())
		{
			const StackElement current = stack.Back();
			stack.RemoveLast();

			const size_t boneIndex = boneToParentTransforms.Size();

			if (current.m_parentIndex != TriangleMesh::k_invalidBoneIndex)
			{
				Math::Matrix4x4 boneGlobalTransform;
				ConvertFBXMatrixToConductorMatrix(
					current.m_node->EvaluateGlobalTransform(FBXSDK_TIME_ZERO), boneGlobalTransform);
				
				boneInverseGlobalTransforms.Add(boneGlobalTransform.CalcInverse());
				
				const Math::Matrix4x4& parentInverseTransform = boneInverseGlobalTransforms[current.m_parentIndex];
				const Math::Matrix4x4 boneToParentTransform = parentInverseTransform * boneGlobalTransform;
				boneToParentTransforms.Add(boneToParentTransform);
			}
			else
			{
				Math::Matrix4x4 boneGlobalTransform;
				ConvertFBXMatrixToConductorMatrix(
					current.m_node->EvaluateGlobalTransform(FBXSDK_TIME_ZERO), boneGlobalTransform);

				// Scale root bones from centimetres to metres.
				const Math::Matrix4x4 centimetresToMetresScale = Math::Matrix4x4::MakeScale(0.01f, 0.01f, 0.01f);
				const Math::Matrix4x4 scaledGlobalTransform = centimetresToMetresScale * boneGlobalTransform;

				boneInverseGlobalTransforms.Add(boneGlobalTransform.CalcInverse());
				boneToParentTransforms.Add(scaledGlobalTransform);
			}
			
			// Store the index of the bone's parent and the bone's name.
			boneParentIndices.Add(static_cast<uint16_t>(current.m_parentIndex));
			boneNames.Emplace(current.m_node->GetName());

			// Find the FbxCluster associated with the bone and copy its weights into its vertices.
			const auto clusterIter = boneNodesToClusters.Find(current.m_node);
			if (clusterIter != boneNodesToClusters.end())
			{
				const FbxCluster& cluster = *clusterIter->second;
				const int numControlPointsInCluster = cluster.GetControlPointIndicesCount();
				const int* const controlPointIndices = cluster.GetControlPointIndices();
				const double* const controlPointWeights = cluster.GetControlPointWeights();
				for (int i = 0; i < numControlPointsInCluster; ++i)
				{
					const int controlPointIndex = controlPointIndices[i];
					const double controlPointWeight = controlPointWeights[i];
					if (controlPointWeight > 1.0f)
					{
						AMP_LOG_WARNING("Vertex weights must be less than or equal to 1.");
						continue;
					}

					// Copy the existing bone weights, add the new bone weight in the right place, and then copy the
					// bone weights back into the vertex.
					uint8_t* const vertex = &vertexData[controlPointIndex * sizeOfVertex];

					struct BoneWeights final
					{
						uint8_t m_boneIndex0;
						uint8_t m_boneWeight0;
						uint8_t m_boneIndex1;
						uint8_t m_boneWeight1;
					};
					BoneWeights vertexBoneWeights;
					memcpy(&vertexBoneWeights, vertex + vertexBoneWeightsOffset, sizeof(BoneWeights));

					if (vertexBoneWeights.m_boneIndex0 == UINT8_MAX)
					{
						vertexBoneWeights.m_boneIndex0 = static_cast<uint8_t>(boneIndex);
						vertexBoneWeights.m_boneWeight0 = static_cast<uint8_t>(controlPointWeight * UINT8_MAX);
						vertexBoneWeights.m_boneIndex1 = static_cast<uint8_t>(boneIndex);
						vertexBoneWeights.m_boneWeight1 = 0;
					}
					else if (vertexBoneWeights.m_boneIndex1 == UINT8_MAX)
					{
						vertexBoneWeights.m_boneIndex1 = static_cast<uint8_t>(boneIndex);
						vertexBoneWeights.m_boneWeight1 = static_cast<uint8_t>(controlPointWeight * UINT8_MAX);
					}
					else
					{
						AMP_LOG_WARNING("Imported FBX files may only skin a vertex to 2 or fewer bones.");
					}

					memcpy(vertex + vertexBoneWeightsOffset, &vertexBoneWeights, sizeof(BoneWeights));
				}
			}

			// Add the bone's children to the stack for traversal.
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
		std::move(boneNames));
	return true;
}
