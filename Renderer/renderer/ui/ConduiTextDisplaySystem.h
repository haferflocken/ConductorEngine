#pragma once

#include <condui/TextDisplayComponent.h>
#include <condui/UITransformComponent.h>
#include <ecs/System.h>

#include <bgfx/bgfx.h>

namespace Renderer::UI
{
/**
 * Makes entities visible by rendering their Condui::TextDisplayComponent.
 */
class TextDisplaySystem final : public ECS::SystemTempl<
	Util::TypeList<Condui::UITransformComponent, Condui::TextDisplayComponent>,
	Util::TypeList<>>
{
public:
	TextDisplaySystem(uint16_t widthPixels, uint16_t heightPixels);
	~TextDisplaySystem();

	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions);

private:
	void RenderCharacterQuad(bgfx::Encoder& encoder,
		const Math::Matrix4x4& uiTransform,
		const float fontScale,
		const int x,
		const int y,
		const char c) const;

	float m_characterWidth;
	float m_characterHeight;

	bgfx::ProgramHandle m_program;
	bgfx::VertexDecl m_vertexDecl;
	bgfx::VertexBufferHandle m_quadVertexBufferHandle;
	bgfx::IndexBufferHandle m_quadIndexBufferHandle;
};
}
