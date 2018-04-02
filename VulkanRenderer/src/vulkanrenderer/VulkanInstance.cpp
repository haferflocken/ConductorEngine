#include <vulkanrenderer/VulkanInstance.h>

#include <vulkanrenderer/InstanceImpl.h>

VulkanRenderer::VulkanInstance::VulkanInstance(
	Collection::LocklessQueue<Client::MessageToRenderInstance>& messagesFromClient,
	Collection::LocklessQueue<Client::InputMessage>& inputToClientMessages,
	const char* const applicationName, const File::Path& vertexShaderFile, const File::Path& fragmentShaderFile)
	: m_internal(Mem::MakeUnique<InstanceImpl>(messagesFromClient, inputToClientMessages, applicationName, vertexShaderFile,
			fragmentShaderFile))
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
