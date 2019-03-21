#include <renderer/PrimitiveRenderer.h>

#include <asset/AssetHandle.h>
#include <asset/AssetManager.h>
#include <image/Colour.h>
#include <mesh/TriangleMesh.h>
#include <renderer/Shader.h>
#include <renderer/VertexDeclarations.h>

namespace Renderer
{
namespace Internal_PrimitiveRenderer
{
class MeshBGFXBuffers final
{
public:
	MeshBGFXBuffers() = default;

	explicit MeshBGFXBuffers(const Mesh::TriangleMesh& mesh)
		: m_vertexBufferHandle(bgfx::createVertexBuffer(
			bgfx::makeRef(&mesh.GetVertexData().Front(), mesh.GetVertexData().Size()), k_posColourVertexDecl))
		, m_indexBufferHandle(bgfx::createIndexBuffer(
			bgfx::makeRef(&mesh.GetTriangleIndices().Front(), mesh.GetTriangleIndices().Size() * sizeof(uint16_t))))
	{}

	void DestroyBuffers()
	{
		if (bgfx::isValid(m_vertexBufferHandle))
		{
			bgfx::destroy(m_vertexBufferHandle);
			m_vertexBufferHandle = BGFX_INVALID_HANDLE;
		}
		if (bgfx::isValid(m_indexBufferHandle))
		{
			bgfx::destroy(m_indexBufferHandle);
			m_indexBufferHandle = BGFX_INVALID_HANDLE;
		}
	}

	bgfx::VertexBufferHandle m_vertexBufferHandle{ BGFX_INVALID_HANDLE };
	bgfx::IndexBufferHandle m_indexBufferHandle{ BGFX_INVALID_HANDLE };
};

const Mesh::TriangleMesh k_quadMesh{
	Collection::Vector<Mesh::PosColourVertex>({
		{ 0.0f, 0.0f, 0.0f, 0xffffffff },
		{ 1.0f, 0.0f, 0.0f, 0xffffffff },
		{ 0.0f, 1.0f, 0.0f, 0xffffffff },
		{ 1.0f, 1.0f, 0.0f, 0xffffffff }}),
	Collection::Vector<uint16_t>({ 0, 1, 2, 2, 1, 3 }) };

const Mesh::TriangleMesh k_pyramidMesh{
	Collection::Vector<Mesh::PosColourVertex>({
		{ -0.5f, -0.5f, -0.5f, 0xffffffff },
		{ 0.5f, -0.5f, -0.5f, 0xffffffff },
		{ -0.5f, 0.5f, -0.5f, 0xffffffff },
		{ 0.5f, 0.5f, -0.5f, 0xffffffff },
		{ 0.0f, 0.0f, 0.5f, 0xffffffff }}),
	Collection::Vector<uint16_t>({ 0, 1, 2, /**/ 2, 1, 3, /**/ 1, 0, 4, /**/ 3, 1, 4, /**/ 2, 3, 4, /**/ 0, 2, 4 }) };

const Mesh::TriangleMesh k_cubeMesh{
	Collection::Vector<Mesh::PosColourVertex>({
		{ -0.5f, -0.5f, -0.5f, 0xffffffff },
		{ 0.5f, -0.5f, -0.5f, 0xffffffff },
		{ -0.5f, 0.5f, -0.5f, 0xffffffff },
		{ 0.5f, 0.5f, -0.5f, 0xffffffff },
		/**/
		{ -0.5f, -0.5f, 0.5f, 0xffffffff },
		{ 0.5f, -0.5f, 0.5f, 0xffffffff },
		{ -0.5f, 0.5f, 0.5f, 0xffffffff },
		{ 0.5f, 0.5f, 0.5f, 0xffffffff }}),
	Collection::Vector<uint16_t>({
	/* Bottom */ 0, 1, 2, 2, 1, 3,
	/* Side A */ 4, 1, 0, 1, 4, 5,
	/* Side B */ 5, 3, 1, 3, 5, 7,
	/* Side C */ 7, 2, 3, 2, 7, 6,
	/* Side D */ 6, 0, 2, 0, 6, 4,
	/* Top    */ 4, 6, 5, 6, 7, 5 }) };

const Mesh::TriangleMesh k_pipeMesh{
	Collection::Vector<Mesh::PosColourVertex>({
		{ -1.0f, -1.0f, 0.0f, 0xffffffff },
		{ 1.0f, -1.0f, 0.0f, 0xffffffff },
		{ -1.0f, 1.0f, 0.0f, 0xffffffff },
		{ 1.0f, 1.0f, 0.0f, 0xffffffff },
		/**/
		{ -1.0f, -1.0f, 1.0f, 0xffffffff },
		{ 1.0f, -1.0f, 1.0f, 0xffffffff },
		{ -1.0f, 1.0f, 1.0f, 0xffffffff },
		{ 1.0f, 1.0f, 1.0f, 0xffffffff }}),
	Collection::Vector<uint16_t>({
	/* Bottom */ 0, 1, 2, 2, 1, 3,
	/* Side A */ 4, 1, 0, 1, 4, 5,
	/* Side B */ 5, 3, 1, 3, 5, 7,
	/* Side C */ 7, 2, 3, 2, 7, 6,
	/* Side D */ 6, 0, 2, 0, 6, 4,
	/* Top    */ 4, 6, 5, 6, 7, 5 }) };

MeshBGFXBuffers g_quadBuffers;
MeshBGFXBuffers g_pyramidBuffers;
MeshBGFXBuffers g_cubeBuffers;
MeshBGFXBuffers g_pipeBuffers;

Asset::AssetHandle<Shader> g_vertexShader;
Asset::AssetHandle<Shader> g_fragmentShader;
bgfx::ProgramHandle g_program = BGFX_INVALID_HANDLE;
bgfx::UniformHandle g_colourUniform = BGFX_INVALID_HANDLE;

void RenderPrimitive(bgfx::Encoder& encoder,
	const bgfx::ViewId viewID,
	const MeshBGFXBuffers& meshBuffers,
	const Math::Matrix4x4& transform,
	const Image::ColourARGB colour)
{
	const Math::Vector4 floatColour = Math::Vector4(colour.r, colour.g, colour.b, colour.a) / UINT8_MAX;

	encoder.setTransform(transform.GetData());
	encoder.setVertexBuffer(0, meshBuffers.m_vertexBufferHandle);
	encoder.setIndexBuffer(meshBuffers.m_indexBufferHandle);
	encoder.setUniform(g_colourUniform, &floatColour);
	encoder.setState(BGFX_STATE_DEFAULT);
	encoder.submit(viewID, g_program);
}
}

bool PrimitiveRenderer::Initialize(Asset::AssetManager& assetManager)
{
	using namespace Internal_PrimitiveRenderer;

	// Initialize the vertex and index buffers.
	g_quadBuffers = MeshBGFXBuffers(k_quadMesh);
	g_pyramidBuffers = MeshBGFXBuffers(k_pyramidMesh);
	g_cubeBuffers = MeshBGFXBuffers(k_cubeMesh);
	g_pipeBuffers = MeshBGFXBuffers(k_pipeMesh);

	// Load the shader program.
	g_vertexShader = assetManager.RequestAsset<Shader>(
		File::MakePath("shaders\\vs_primitive.bin"), Asset::LoadingMode::Immediate);
	g_fragmentShader = assetManager.RequestAsset<Shader>(
		File::MakePath("shaders\\fs_primitive.bin"), Asset::LoadingMode::Immediate);

	const Shader* const vertexShader = g_vertexShader.TryGetAsset();
	const Shader* const fragmentShader = g_fragmentShader.TryGetAsset();

	if (vertexShader != nullptr && fragmentShader != nullptr)
	{
		g_program = bgfx::createProgram(vertexShader->GetShaderHandle(), fragmentShader->GetShaderHandle(), false);
		g_colourUniform = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
		return bgfx::isValid(g_program);
	}
	return false;
}

void PrimitiveRenderer::Shutdown()
{
	using namespace Internal_PrimitiveRenderer;

	if (bgfx::isValid(g_colourUniform))
	{
		bgfx::destroy(g_colourUniform);
	}
	if (bgfx::isValid(g_program))
	{
		bgfx::destroy(g_program);
	}

	g_fragmentShader = Asset::AssetHandle<Shader>();
	g_vertexShader = Asset::AssetHandle<Shader>();

	g_pipeBuffers.DestroyBuffers();
	g_cubeBuffers.DestroyBuffers();
	g_pyramidBuffers.DestroyBuffers();
	g_quadBuffers.DestroyBuffers();
}

void PrimitiveRenderer::DrawQuad(bgfx::Encoder& encoder,
	const bgfx::ViewId viewID,
	const Math::Matrix4x4& transform,
	const float width,
	const float height,
	const Image::ColourARGB colour)
{
	using namespace Internal_PrimitiveRenderer;

	const Math::Matrix4x4 m = transform * Math::Matrix4x4::MakeScale(width, height, 1.0f);
	RenderPrimitive(encoder, viewID, g_quadBuffers, m, colour);
}

void PrimitiveRenderer::DrawPyramid(bgfx::Encoder& encoder,
	const bgfx::ViewId viewID,
	const Math::Matrix4x4& transform,
	const Math::Vector3& scale,
	const Image::ColourARGB colour)
{
	using namespace Internal_PrimitiveRenderer;

	const Math::Matrix4x4 m = transform * Math::Matrix4x4::MakeScale(scale.x, scale.y, scale.z);
	RenderPrimitive(encoder, viewID, g_pyramidBuffers, m, colour);
}

void PrimitiveRenderer::DrawCube(bgfx::Encoder& encoder,
	const bgfx::ViewId viewID,
	const Math::Matrix4x4& transform,
	const Math::Vector3& scale,
	const Image::ColourARGB colour)
{
	using namespace Internal_PrimitiveRenderer;

	const Math::Matrix4x4 m = transform * Math::Matrix4x4::MakeScale(scale.x, scale.y, scale.z);
	RenderPrimitive(encoder, viewID, g_cubeBuffers, m, colour);
}

void PrimitiveRenderer::DrawPipe(bgfx::Encoder& encoder,
	const bgfx::ViewId viewID,
	const Math::Vector3& startPos,
	const Math::Vector3& endPos,
	const float radius,
	const Image::ColourARGB colour)
{
	using namespace Internal_PrimitiveRenderer;

	const Math::Vector3 offset = endPos - startPos;
	const float length = offset.Length();
	if (length < 0.00001f)
	{
		return;
	}
	const Math::Vector3 direction = offset / length;

	const Math::Vector3 upVector = (direction.Dot(Math::Vector3(0.0f, 1.0f, 0.0f)) < 0.99f)
		? Math::Vector3(0.0f, 1.0f, 0.0f) : Math::Vector3(1.0f, 0.0f, 0.0f);

	const auto translation = Math::Matrix4x4::MakeTranslation(startPos.x, startPos.y, startPos.z);
	const auto orientation = Math::Matrix4x4::MakeOrientZAlong(upVector, direction);
	const auto scale = Math::Matrix4x4::MakeScale(radius, radius, length);
	const Math::Matrix4x4 m = translation * orientation * scale;

	RenderPrimitive(encoder, viewID, g_pipeBuffers, m, colour);
}
}
