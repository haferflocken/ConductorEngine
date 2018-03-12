#include <vulkanrenderer/VulkanInstance.h>

#include <vulkanrenderer/InstanceImpl.h>

using namespace VulkanRenderer;

VulkanInstance::VulkanInstance(const char* const applicationName, const File::Path& vertexShaderFile,
	const File::Path& fragmentShaderFile)
	: m_internal(Mem::MakeUnique<InstanceImpl>(applicationName, vertexShaderFile, fragmentShaderFile))
{
}

GameBase::IRenderInstance::Status VulkanInstance::Update()
{
	return m_internal->Update();
}

VulkanInstance::~VulkanInstance()
{
}

VulkanInstance::Status VulkanInstance::GetStatus() const
{
	return m_internal->GetStatus();
}
