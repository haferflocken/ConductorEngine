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

	MeshBGFXBuffers(const Mesh::TriangleMesh& mesh)
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

MeshBGFXBuffers g_quadBuffers;
MeshBGFXBuffers g_pyramidBuffers;

Asset::AssetHandle<Shader> g_vertexShader;
Asset::AssetHandle<Shader> g_fragmentShader;
bgfx::ProgramHandle g_program = BGFX_INVALID_HANDLE;
bgfx::UniformHandle g_colourUniform = BGFX_INVALID_HANDLE;
}

bool PrimitiveRenderer::Initialize(Asset::AssetManager& assetManager)
{
	using namespace Internal_PrimitiveRenderer;

	// Initialize the vertex and index buffers.
	g_quadBuffers = MeshBGFXBuffers(k_quadMesh);
	g_pyramidBuffers = MeshBGFXBuffers(k_pyramidMesh);

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

	const Math::Vector4 floatColour = Math::Vector4(colour.r, colour.g, colour.b, colour.a) / UINT8_MAX;

	const Math::Matrix4x4 m = transform * Math::Matrix4x4::MakeScale(width, height, 1.0f);

	encoder.setTransform(m.GetData());
	encoder.setVertexBuffer(0, g_quadBuffers.m_vertexBufferHandle);
	encoder.setIndexBuffer(g_quadBuffers.m_indexBufferHandle);
	encoder.setUniform(g_colourUniform, &floatColour);
	encoder.setState(BGFX_STATE_DEFAULT);
	encoder.submit(viewID, g_program);
}

void PrimitiveRenderer::DrawPyramid(bgfx::Encoder& encoder,
	const bgfx::ViewId viewID,
	const Math::Matrix4x4& transform,
	const Math::Vector3& scale,
	const Image::ColourARGB colour)
{
	using namespace Internal_PrimitiveRenderer;

	const Math::Vector4 floatColour = Math::Vector4(colour.r, colour.g, colour.b, colour.a) / UINT8_MAX;

	const Math::Matrix4x4 m = transform * Math::Matrix4x4::MakeScale(scale.x, scale.y, scale.z);

	encoder.setTransform(m.GetData());
	encoder.setVertexBuffer(0, g_pyramidBuffers.m_vertexBufferHandle);
	encoder.setIndexBuffer(g_pyramidBuffers.m_indexBufferHandle);
	encoder.setUniform(g_colourUniform, &floatColour);
	encoder.setState(BGFX_STATE_DEFAULT);
	encoder.submit(viewID, g_program);
}
}
