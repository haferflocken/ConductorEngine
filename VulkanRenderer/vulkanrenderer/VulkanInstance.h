#pragma once

#include <client/IRenderInstance.h>
#include <file/Path.h>
#include <mem/UniquePtr.h>

namespace Client { struct InputMessage; struct MessageToRenderInstance; }
namespace Collection { template <typename T> class LocklessQueue; }

namespace VulkanRenderer
{
class InstanceImpl;

/**
 * A ConcurrentGame window that renders the game using Vulkan.
 */
class VulkanInstance final : public Client::IRenderInstance
{
public:
	// Constructor and destructor implemented in the cpp file so that unique_ptr can be declared on a forward declaration.
	explicit VulkanInstance(Collection::LocklessQueue<Client::MessageToRenderInstance>& messagesFromClient,
		Collection::LocklessQueue<Client::InputMessage>& inputToClientMessages,
		const char* const applicationName, const File::Path& vertexShaderFile, const File::Path& fragmentShaderFile);
	~VulkanInstance();

	virtual Status GetStatus() const override;

	virtual Status Update() override;

private:
	// Entirely hide the implementation so that files which include this don't get any vulkan includes.
	Mem::UniquePtr<InstanceImpl> m_internal;
};
}
