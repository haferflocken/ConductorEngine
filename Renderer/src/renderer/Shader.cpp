#include <renderer/Shader.h>

#include <file/FullFileReader.h>

#include <bgfx/bgfx.h>

namespace Renderer
{
bool Shader::TryLoad(const File::Path& filePath, Shader* destination)
{
	std::string binaryString = File::ReadFullTextFile(filePath);
	if (binaryString.empty())
	{
		return false;
	}

	const bgfx::ShaderHandle shaderHandle = bgfx::createShader(
		bgfx::makeRef(binaryString.data(), static_cast<uint32_t>(binaryString.size())));
	if (!bgfx::isValid(shaderHandle))
	{
		return false;
	}
	destination = new (destination) Shader(std::move(binaryString), shaderHandle);
	return true;
}

Shader::Shader(std::string&& binaryString, bgfx::ShaderHandle shaderHandle)
	: m_binaryString(std::move(binaryString))
	, m_shaderHandle(Mem::MakeUnique<bgfx::ShaderHandle>(shaderHandle))
{}

Shader::~Shader()
{
	bgfx::destroy(*m_shaderHandle);
}
}
