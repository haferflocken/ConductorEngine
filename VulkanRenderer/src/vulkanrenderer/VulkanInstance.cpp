#include <vulkanrenderer/VulkanInstance.h>

#include <vulkanrenderer/InstanceImpl.h>

VulkanRenderer::VulkanInstance::VulkanInstance(const char* const applicationName, const File::Path& vertexShaderFile,
	const File::Path& fragmentShaderFile)
	: m_internal(Mem::MakeUnique<InstanceImpl>(applicationName, vertexShaderFile, fragmentShaderFile))
{
}

Client::IRenderInstance::Status VulkanRenderer::VulkanInstance::Update()
{
	return m_internal->Update();
}

VulkanRenderer::VulkanInstance::~VulkanInstance()
{
}

VulkanRenderer::VulkanInstance::Status VulkanRenderer::VulkanInstance::GetStatus() const
{
	return m_internal->GetStatus();
}
