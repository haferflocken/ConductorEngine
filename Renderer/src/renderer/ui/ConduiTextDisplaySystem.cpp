#include <renderer/ui/ConduiTextDisplaySystem.h>

#include <mesh/StaticMesh.h>
#include <renderer/ViewIDs.h>

namespace Renderer::UI
{
namespace Internal_TextDisplaySystem
{
constexpr float k_characterWidthPixels = 9;
constexpr float k_characterHeightPixels = 16;

const Mesh::StaticMesh k_characterQuad{
	Collection::Vector<Mesh::Vertex>({
		{ 0.0f, 0.0f, 0.0f, 0xff0000ff },
		{ 1.0f, 0.0f, 0.0f, 0xffff0000 },
		{ 0.0f, 1.0f, 0.0f, 0xff00ff00 },
		{ 1.0f, 1.0f, 0.0f, 0xffffff00 }}),
	Collection::Vector<uint16_t>({ 0, 1, 2, 2, 1, 3 }) };
}

TextDisplaySystem::TextDisplaySystem(uint16_t widthPixels, uint16_t heightPixels)
	: SystemTempl()
	, m_characterWidth(Internal_TextDisplaySystem::k_characterWidthPixels / widthPixels)
	, m_characterHeight(Internal_TextDisplaySystem::k_characterHeightPixels / heightPixels)
	, m_program()
	, m_vertexDecl()
	, m_quadVertexBufferHandle()
	, m_quadIndexBufferHandle()
{
	using namespace Internal_TextDisplaySystem;

	//m_program = bgfx::createProgram(vertexShader, fragmentShader, true);

	m_vertexDecl.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Uint8)
		.end();

	m_quadVertexBufferHandle = bgfx::createVertexBuffer(
		bgfx::makeRef(&k_characterQuad.GetVertices().Front(),
			k_characterQuad.GetVertices().Size() * sizeof(Mesh::Vertex)),
		m_vertexDecl);

	m_quadIndexBufferHandle = bgfx::createIndexBuffer(
		bgfx::makeRef(&k_characterQuad.GetTriangleIndices().Front(),
			k_characterQuad.GetTriangleIndices().Size() * sizeof(uint16_t)));
}

TextDisplaySystem::~TextDisplaySystem()
{
	bgfx::destroy(m_quadIndexBufferHandle);
	bgfx::destroy(m_quadVertexBufferHandle);
	//bgfx::destroy(m_program);
}

void TextDisplaySystem::Update(const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions)
{
	bgfx::Encoder* const encoder = bgfx::begin();
	if (encoder == nullptr)
	{
		return;
	}

	// Render each text display entity as a series of textured quads.
	for (const auto& ecsGroup : ecsGroups)
	{
		const auto& transformComponent = ecsGroup.Get<const Condui::UITransformComponent>();
		const auto& textComponent = ecsGroup.Get<const Condui::TextDisplayComponent>();

		int x = 0;
		int y = -1; // We begin at -1 because characters render from the bottom up.
		char previous = '#';
		
		for (const auto& c : textComponent.m_string)
		{
			switch (c)
			{
			case ' ':
			{
				x += 1;
				break;
			}
			case '\t':
			{
				const int xAt4 = x & ~(0b11);
				x = xAt4 + 4;
				break;
			}
			case '\r':
			{
				x = 0;
				y -= 1;
				break;
			}
			case '\n':
			{
				if (previous != '\r')
				{
					x = 0;
					y -= 1;
				}
				break;
			}
			default:
			{
				RenderCharacterQuad(*encoder, transformComponent.m_uiTransform, textComponent.m_fontScale, x, y, c);
				x += 1;
				break;
			}
			}
			previous = c;
		}
	}
}

void TextDisplaySystem::RenderCharacterQuad(bgfx::Encoder& encoder,
	const Math::Matrix4x4& uiTransform,
	const float fontScale,
	const int x,
	const int y,
	const char c) const
{
	const float scaledCharacterWidth = m_characterWidth * fontScale;
	const float scaledCharacterHeight = m_characterHeight * fontScale;

	Math::Vector3 offset{ 0.0f, 0.0f, 0.0f };
	offset.x = x * scaledCharacterWidth;
	offset.y = y * scaledCharacterHeight;

	Math::Matrix4x4 characterTransform;
	characterTransform.SetTranslation(offset);
	characterTransform.SetScale(Math::Vector3(scaledCharacterWidth, scaledCharacterHeight, 0.0f));

	const Math::Matrix4x4 transform = uiTransform * characterTransform;

	encoder.setTransform(transform.GetData());
	encoder.setVertexBuffer(0, m_quadVertexBufferHandle);
	encoder.setIndexBuffer(m_quadIndexBufferHandle);
	encoder.setState(BGFX_STATE_DEFAULT);
	encoder.submit(k_uiViewID, m_program);
}
}
