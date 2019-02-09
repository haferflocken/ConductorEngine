#include <renderer/PrimitiveRenderer.h>

#include <asset/AssetHandle.h>
#include <asset/AssetManager.h>
#include <image/Colour.h>
#include <mesh/StaticMesh.h>
#include <renderer/Shader.h>
#include <renderer/VertexDeclarations.h>

namespace Renderer
{
namespace Internal_PrimitiveRenderer
{
const Mesh::StaticMesh k_quadMesh{
	Collection::Vector<Mesh::PosColourVertex>({
		{ 0.0f, 0.0f, 0.0f, 0xffffffff },
		{ 1.0f, 0.0f, 0.0f, 0xffffffff },
		{ 0.0f, 1.0f, 0.0f, 0xffffffff },
		{ 1.0f, 1.0f, 0.0f, 0xffffffff }}),
	Collection::Vector<uint16_t>({ 0, 1, 2, 2, 1, 3 }) };

bgfx::VertexBufferHandle g_quadVertexBufferHandle = BGFX_INVALID_HANDLE;
bgfx::IndexBufferHandle g_quadIndexBufferHandle = BGFX_INVALID_HANDLE;

Asset::AssetHandle<Shader> g_vertexShader;
Asset::AssetHandle<Shader> g_fragmentShader;
bgfx::ProgramHandle g_program = BGFX_INVALID_HANDLE;
bgfx::UniformHandle g_colourUniform = BGFX_INVALID_HANDLE;
}

bool PrimitiveRenderer::Initialize(Asset::AssetManager& assetManager)
{
	using namespace Internal_PrimitiveRenderer;

	// Initialize the vertex and index buffers.
	g_quadVertexBufferHandle = bgfx::createVertexBuffer(
		bgfx::makeRef(&k_quadMesh.GetVertexData().Front(), k_quadMesh.GetVertexData().Size()),
		k_posColourVertexDecl);
	g_quadIndexBufferHandle = bgfx::createIndexBuffer(
		bgfx::makeRef(&k_quadMesh.GetTriangleIndices().Front(),
			k_quadMesh.GetTriangleIndices().Size() * sizeof(uint16_t)));

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

	bgfx::destroy(g_quadIndexBufferHandle);
	bgfx::destroy(g_quadVertexBufferHandle);
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
	encoder.setVertexBuffer(0, g_quadVertexBufferHandle);
	encoder.setIndexBuffer(g_quadIndexBufferHandle);
	encoder.setUniform(g_colourUniform, &floatColour);
	encoder.setState(BGFX_STATE_DEFAULT);
	encoder.submit(viewID, g_program);
}
}
