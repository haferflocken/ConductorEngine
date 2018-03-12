#pragma once

#include <vulkanrenderer/vulkanobjects/InfoObjects.h>
#include <vulkanrenderer/VulkanPlatform.h>
#include <vulkanrenderer/VulkanUtils.h>

#include <file/FullFileReader.h>
#include <file/Path.h>

namespace VulkanRenderer
{
namespace VulkanObjects
{
class ShadersObject
{
public:
	ShadersObject(const vk::Device& device, const File::Path& vertexShaderFile, const File::Path& fragmentShaderFile)
		: m_deviceRef(device)
	{
		Utils::InitShaderCompiler();

		// Compile the shader files to SPIRV.
		const std::string vertexShaderSource = File::ReadFullTextFile(vertexShaderFile);
		const std::vector<unsigned int> vertexSPIRV =
			Utils::CompileShader(vk::ShaderStageFlagBits::eVertex, vertexShaderSource);
		
		const std::string fragmentShaderSource = File::ReadFullTextFile(fragmentShaderFile);
		const std::vector<unsigned int> fragmentSPIRV =
			Utils::CompileShader(vk::ShaderStageFlagBits::eFragment, fragmentShaderSource);

		// Create shader modules.
		const auto vertexModuleCreateInfo = vk::ShaderModuleCreateInfo()
			.setCodeSize(vertexSPIRV.size() * sizeof(unsigned int))
			.setPCode(vertexSPIRV.data());

		m_vertexShader = vk::PipelineShaderStageCreateInfo()
			.setStage(vk::ShaderStageFlagBits::eVertex)
			.setPName("main")
			.setModule(device.createShaderModule(vertexModuleCreateInfo));

		const auto fragmentModuleCreateInfo = vk::ShaderModuleCreateInfo()
			.setCodeSize(fragmentSPIRV.size() * sizeof(unsigned int))
			.setPCode(fragmentSPIRV.data());

		m_fragmentShader = vk::PipelineShaderStageCreateInfo()
			.setStage(vk::ShaderStageFlagBits::eFragment)
			.setPName("main")
			.setModule(device.createShaderModule(fragmentModuleCreateInfo));

		Utils::FinalizeShaderCompiler();
	}

	~ShadersObject()
	{
		m_deviceRef.destroyShaderModule(m_vertexShader.module);
		m_deviceRef.destroyShaderModule(m_fragmentShader.module);
	}

private:
	const vk::Device& m_deviceRef;
	vk::PipelineShaderStageCreateInfo m_vertexShader;
	vk::PipelineShaderStageCreateInfo m_fragmentShader;
};
}
}
