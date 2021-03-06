#include <renderer/debug/SkeletonDebugRenderSystem.h>

#include <image/Colour.h>
#include <math/MathConstants.h>

#include <renderer/PrimitiveRenderer.h>
#include <renderer/ui/TextRenderer.h>
#include <renderer/ViewIDs.h>

namespace Renderer::Debug
{
void SkeletonDebugRenderSystem::Update(const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const
{
	bgfx::Encoder* const encoder = bgfx::begin();
	if (encoder == nullptr)
	{
		return;
	}

	for (const auto& ecsGroup : ecsGroups)
	{
		const auto& meshComponent = ecsGroup.Get<const Mesh::MeshComponent>();
		const Mesh::TriangleMesh* const mesh = meshComponent.m_meshHandle.TryGetAsset();
		if (mesh == nullptr)
		{
			continue;
		}

		const auto& boneParentIndices = mesh->GetBoneParentIndices();
		const auto& boneNames = mesh->GetBoneNames();
		AMP_FATAL_ASSERT(boneParentIndices.Size() == meshComponent.m_boneToWorldMatrices.Size(), "");
		AMP_FATAL_ASSERT(boneNames.Size() == meshComponent.m_boneToWorldMatrices.Size(), "");

		for (size_t i = 0, iEnd = boneParentIndices.Size(); i < iEnd; ++i)
		{
			const Math::Matrix4x4& boneTransform = meshComponent.m_boneToWorldMatrices[i];
			const char* const boneName = boneNames[i].c_str();
			
			PrimitiveRenderer::DrawCube(
				*encoder, k_sceneViewID, boneTransform, Math::Vector3(0.5f, 0.5f, 0.5f), Image::ColoursARBG::k_green);

			m_textRenderer.RequestFont(m_codePage, m_characterWidthPixels, m_characterHeightPixels);
			m_textRenderer.SubmitCameraFacingText(
				*encoder,
				k_sceneViewID,
				boneTransform.GetTranslation(),
				Image::ColoursARBG::k_black,
				m_codePage,
				boneName,
				0.5f);
			
			const uint16_t& parentIndex = boneParentIndices[i];
			if (parentIndex != Mesh::TriangleMesh::k_invalidBoneIndex)
			{
				const Math::Matrix4x4& parentTransform = meshComponent.m_boneToWorldMatrices[parentIndex];
				PrimitiveRenderer::DrawPipe(
					*encoder,
					k_sceneViewID,
					boneTransform.GetTranslation(),
					parentTransform.GetTranslation(),
					0.1f,
					Image::ColoursARBG::k_red);
			}
		}
	}

	bgfx::end(encoder);
}
}
