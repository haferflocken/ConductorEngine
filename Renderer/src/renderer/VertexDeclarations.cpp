#include <renderer/VertexDeclarations.h>

#include <mesh/Vertex.h>

bgfx::VertexDecl Renderer::MakeBGFXVertexDecl(const Mesh::ExpandedVertexDeclaration& vertexDeclaration)
{
	bgfx::VertexDecl result;
	result.begin();
	for (size_t i = 0, iEnd = vertexDeclaration.m_numAttributes; i < iEnd; ++i)
	{
		const Mesh::VertexAttribute attribute = vertexDeclaration.m_attributes[i];
		switch (attribute)
		{
		case Mesh::VertexAttribute::Invalid:
		{
			AMP_LOG_ERROR("Encountered invalid vertex attribute when creating a bgfx::VertexDecl.");
			result.end();
			return result;
		}
		case Mesh::VertexAttribute::Position:
		{
			result.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
			break;
		}
		case Mesh::VertexAttribute::Normal:
		{
			result.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float);
			break;
		}
		case Mesh::VertexAttribute::TextureCoords0:
		{
			result.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
			break;
		}
		case Mesh::VertexAttribute::TextureCoords1:
		{
			result.add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float);
			break;
		}
		case Mesh::VertexAttribute::Colour0:
		{
			result.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true);
			break;
		}
		case Mesh::VertexAttribute::Colour1:
		{
			result.add(bgfx::Attrib::Color1, 4, bgfx::AttribType::Uint8, true);
			break;
		}
		case Mesh::VertexAttribute::WeightGroup:
		{
			result.add(bgfx::Attrib::Weight, 1, bgfx::AttribType::Float);
			break;
		}
		default:
		{
			static_assert(static_cast<size_t>(Mesh::VertexAttribute::Count) == 8,
				"All vertex attribute types must be handled here!");
			AMP_LOG_ERROR("Encountered an unknown vertex attribute when creating a bgfx::VertexDecl.");
			result.end();
			return result;
		}
		}
	}
	result.end();
	return result;
}
