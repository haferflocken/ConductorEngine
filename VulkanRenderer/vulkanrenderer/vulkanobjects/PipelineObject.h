#pragma once

#include <vulkanrenderer/VulkanPlatform.h>

namespace VulkanRenderer
{
namespace VulkanObjects
{
class PipelineObject
{
public:
	PipelineObject(const vk::Device& device)
		: m_deviceRef(device)
		, m_cache()
		, m_pipeline()
	{
		// Create the pipeline cache.
		const auto cacheCreateInfo = vk::PipelineCacheCreateInfo();
		m_cache = device.createPipelineCache(cacheCreateInfo);

		// TODO Create the pipeline.
	}

	~PipelineObject()
	{
		// TODO m_deviceRef.destroyPipeline(m_pipeline);
		m_deviceRef.destroyPipelineCache(m_cache);
	}

private:
	const vk::Device& m_deviceRef;
	vk::PipelineCache m_cache;
	vk::Pipeline m_pipeline;
};
}
}
