#pragma once

#include <file/Path.h>
#include <client/IRenderInstance.h>
#include <mem/UniquePtr.h>

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
	explicit VulkanInstance(const char* const applicationName, const File::Path& vertexShaderFile,
		const File::Path& fragmentShaderFile);
	~VulkanInstance();

	virtual Status GetStatus() const override;

	virtual Status Update() override;

private:
	// Entirely hide the implementation so that files which include this don't get any vulkan includes.
	Mem::UniquePtr<InstanceImpl> m_internal;
};
}
