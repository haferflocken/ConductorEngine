#pragma once

#include <file/Path.h>
#include <mem/UniquePtr.h>

#include <string>

namespace bgfx { struct ShaderHandle; }

namespace Renderer
{
/**
 * A binary shader asset.
 */
class Shader final
{
public:
	static bool TryLoad(const File::Path& filePath, Shader* destination);

	Shader(std::string&& binaryString, bgfx::ShaderHandle shaderHandle);
	~Shader();

	const bgfx::ShaderHandle& GetShaderHandle() const { return *m_shaderHandle; }

private:
	std::string m_binaryString;
	// Kept in a UniquePtr to prevent this header from requiring bgfx.h.
	Mem::UniquePtr<bgfx::ShaderHandle> m_shaderHandle;
};
}
