#include <vulkanrenderer/VulkanUtils.h>

#include <vulkanrenderer/InstanceData.h>

#ifdef __ANDROID__
// Android specific include files.
#include <unordered_map>

// Header files.
#include <android_native_app_glue.h>
#include "shaderc/shaderc.hpp"
// Static variable that keeps ANativeWindow and asset manager instances.
static android_app* Android_application = nullptr;
#else
#include <glslang/SPIRV/GlslangToSpv.h>
#endif

#include <assert.h>

const vk::ComponentMapping VulkanRenderer::Utils::k_colorImageComponentMapping =
	vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB,
		vk::ComponentSwizzle::eA);

const vk::ImageSubresourceRange VulkanRenderer::Utils::k_colorSubresourceRange =
	vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

const vk::ImageSubresourceRange VulkanRenderer::Utils::k_depthSubresourceRange =
	vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1);

const vk::Format VulkanRenderer::Utils::k_depthFormat = vk::Format::eD16Unorm;

const vk::SampleCountFlagBits VulkanRenderer::Utils::k_numSamples = vk::SampleCountFlagBits::e1;

const size_t VulkanRenderer::Utils::k_numDescriptorSets = 1;

VulkanRenderer::VulkanObjects::InstanceInfo VulkanRenderer::Utils::MakeInstanceInfo()
{
	VulkanObjects::InstanceInfo instanceInfo;

	// Init the extension names.
	instanceInfo.extensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	instanceInfo.extensionNames.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MIR_KHR)
	instanceInfo.extensionNames.push_back(VK_KHR_MIR_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	instanceInfo.extensionNames.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
	instanceInfo.extensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	instanceInfo.extensionNames.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

	return instanceInfo;
}

VulkanRenderer::VulkanObjects::WindowInfo VulkanRenderer::Utils::MakeWindowInfo(const int32_t desiredWidth, const int32_t desiredHeight)
{
	VulkanObjects::WindowInfo windowInfo;

#ifdef __ANDROID__
	AndroidGetWindowSize(&windowInfo.width, &windowInfo.height);
#else
	windowInfo.width = desiredWidth;
	windowInfo.height = desiredHeight;
#endif

	return windowInfo;
}

VulkanRenderer::VulkanObjects::PhysicalDevicesInfo VulkanRenderer::Utils::MakePhysicalDevicesInfo(const vk::Instance& instance,
	const vk::SurfaceKHR& surface)
{
	VulkanObjects::PhysicalDevicesInfo physicalDevicesInfo;

	physicalDevicesInfo.gpus = instance.enumeratePhysicalDevices();
	if (physicalDevicesInfo.gpus.empty())
	{
		throw std::runtime_error("Cannot create a Vulkan renderer without any GPUs.");
	}

	// The primary GPU that we will use is just the first one in the list. By naming it, it is easier to change later.
	physicalDevicesInfo.primaryGPU = physicalDevicesInfo.gpus.front();

	// Get the memory properties of the primary GPU.
	physicalDevicesInfo.memoryProperties = physicalDevicesInfo.primaryGPU.getMemoryProperties();

	// Determine the color format and color space that the device supports for this surface.
	const std::vector<vk::SurfaceFormatKHR> surfaceFormats = physicalDevicesInfo.primaryGPU.getSurfaceFormatsKHR(surface);
	if (surfaceFormats.size() == 1 && surfaceFormats.front().format == vk::Format::eUndefined)
	{
		// If there is only one format and it is undefined, we get to pick.
		physicalDevicesInfo.colorFormat = vk::Format::eB8G8R8A8Unorm;
	}
	else
	{
		physicalDevicesInfo.colorFormat = surfaceFormats.front().format;
	}
	physicalDevicesInfo.colorSpace = surfaceFormats.front().colorSpace;

	// Get the queue family properties so we can search them.
	physicalDevicesInfo.queueProperties = physicalDevicesInfo.primaryGPU.getQueueFamilyProperties();

	// Search for a graphics queue family and a present queue family among the queue families.
	physicalDevicesInfo.graphicsQueueFamilyIndex = std::numeric_limits<uint32_t>::max();
	physicalDevicesInfo.presentQueueFamilyIndex = std::numeric_limits<uint32_t>::max();
	for (size_t i = 0, iEnd = physicalDevicesInfo.queueProperties.size(); i < iEnd; ++i)
	{
		const auto& queueFamilyProperties = physicalDevicesInfo.queueProperties[i];
		if (queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			physicalDevicesInfo.graphicsQueueFamilyIndex = static_cast<uint32_t>(i);
		}

		const vk::Bool32 supportsPresent = physicalDevicesInfo.primaryGPU.getSurfaceSupportKHR(i, surface);
		if (supportsPresent)
		{
			// By always reassigning when we find a present queue family, we make it so that if a queue family supports both
			// graphics and present, the graphics and present queue families will be the same.
			physicalDevicesInfo.presentQueueFamilyIndex = static_cast<uint32_t>(i);
		}

		// Once we have found both a graphics queue family and a present queue family, we can stop searching.
		if (physicalDevicesInfo.graphicsQueueFamilyIndex != std::numeric_limits<uint32_t>::max()
			&& physicalDevicesInfo.presentQueueFamilyIndex != std::numeric_limits<uint32_t>::max())
		{
			break;
		}
	}

	if (physicalDevicesInfo.graphicsQueueFamilyIndex == std::numeric_limits<uint32_t>::max())
	{
		throw std::runtime_error("Failed to find a graphics queue family.");
	}

	if (physicalDevicesInfo.presentQueueFamilyIndex == std::numeric_limits<uint32_t>::max())
	{
		throw std::runtime_error("Failed to find a present queue family.");
	}

	// Make the logical device aware that it must support swapchains.
	physicalDevicesInfo.extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	return physicalDevicesInfo;
}

uint32_t VulkanRenderer::Utils::FindMemoryType(const VulkanObjects::PhysicalDevicesInfo& physicalDevicesInfo,
	uint32_t typeBits, const vk::MemoryPropertyFlags requirementsMask)
{
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < physicalDevicesInfo.memoryProperties.memoryTypeCount; ++i)
	{
		if ((typeBits & 1) == 1)
		{
			// Type is available, does it match user properties?
			if ((physicalDevicesInfo.memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)
			{
				return i;
			}
		}
		typeBits >>= 1;
	}
	// No memory types matched, return failure
	return std::numeric_limits<uint32_t>::max();
}

void VulkanRenderer::Utils::SetImageLayout(const vk::CommandBuffer& commandBuffer, const vk::Image& image,
	const vk::ImageAspectFlags aspectMask, const vk::ImageLayout oldImageLayout, const vk::ImageLayout newImageLayout)
{
	// Depends on the graphics queue being initialized.
	
	const vk::ImageSubresourceRange imageMemoryBarrierSubresourceRange = vk::ImageSubresourceRange()
		.setAspectMask(aspectMask)
		.setBaseMipLevel(0)
		.setLevelCount(1)
		.setBaseArrayLayer(0)
		.setLayerCount(1);

	vk::ImageMemoryBarrier image_memory_barrier = vk::ImageMemoryBarrier()
		.setSrcAccessMask(vk::AccessFlags())
		.setDstAccessMask(vk::AccessFlags())
		.setOldLayout(oldImageLayout)
		.setNewLayout(newImageLayout)
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setImage(image)
		.setSubresourceRange(imageMemoryBarrierSubresourceRange);

	switch (oldImageLayout)
	{
	case vk::ImageLayout::eColorAttachmentOptimal:
	{
		image_memory_barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		break;
	}
	case vk::ImageLayout::eTransferDstOptimal:
	{
		image_memory_barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		break;
	}
	case vk::ImageLayout::ePreinitialized:
	{
		image_memory_barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
		break;
	}
	}

	switch (newImageLayout)
	{
	case vk::ImageLayout::eTransferDstOptimal:
	{
		image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		break;
	}
	case vk::ImageLayout::eTransferSrcOptimal:
	{
		image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
		break;
	}
	case vk::ImageLayout::eShaderReadOnlyOptimal:
	{
		image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		break;
	}
	case vk::ImageLayout::eColorAttachmentOptimal:
	{
		image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		break;
	}
	case vk::ImageLayout::eDepthStencilAttachmentOptimal:
	{
		image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		break;
	}
	}

	const vk::PipelineStageFlags srcStages = vk::PipelineStageFlagBits::eTopOfPipe;
	const vk::PipelineStageFlags destStages = vk::PipelineStageFlagBits::eTopOfPipe;

	commandBuffer.pipelineBarrier(srcStages, destStages, vk::DependencyFlags(), {}, {}, { image_memory_barrier });
}

#ifndef __ANDROID__
namespace
{
void InitResources(TBuiltInResource& Resources)
{
	Resources.maxLights = 32;
	Resources.maxClipPlanes = 6;
	Resources.maxTextureUnits = 32;
	Resources.maxTextureCoords = 32;
	Resources.maxVertexAttribs = 64;
	Resources.maxVertexUniformComponents = 4096;
	Resources.maxVaryingFloats = 64;
	Resources.maxVertexTextureImageUnits = 32;
	Resources.maxCombinedTextureImageUnits = 80;
	Resources.maxTextureImageUnits = 32;
	Resources.maxFragmentUniformComponents = 4096;
	Resources.maxDrawBuffers = 32;
	Resources.maxVertexUniformVectors = 128;
	Resources.maxVaryingVectors = 8;
	Resources.maxFragmentUniformVectors = 16;
	Resources.maxVertexOutputVectors = 16;
	Resources.maxFragmentInputVectors = 15;
	Resources.minProgramTexelOffset = -8;
	Resources.maxProgramTexelOffset = 7;
	Resources.maxClipDistances = 8;
	Resources.maxComputeWorkGroupCountX = 65535;
	Resources.maxComputeWorkGroupCountY = 65535;
	Resources.maxComputeWorkGroupCountZ = 65535;
	Resources.maxComputeWorkGroupSizeX = 1024;
	Resources.maxComputeWorkGroupSizeY = 1024;
	Resources.maxComputeWorkGroupSizeZ = 64;
	Resources.maxComputeUniformComponents = 1024;
	Resources.maxComputeTextureImageUnits = 16;
	Resources.maxComputeImageUniforms = 8;
	Resources.maxComputeAtomicCounters = 8;
	Resources.maxComputeAtomicCounterBuffers = 1;
	Resources.maxVaryingComponents = 60;
	Resources.maxVertexOutputComponents = 64;
	Resources.maxGeometryInputComponents = 64;
	Resources.maxGeometryOutputComponents = 128;
	Resources.maxFragmentInputComponents = 128;
	Resources.maxImageUnits = 8;
	Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
	Resources.maxCombinedShaderOutputResources = 8;
	Resources.maxImageSamples = 0;
	Resources.maxVertexImageUniforms = 0;
	Resources.maxTessControlImageUniforms = 0;
	Resources.maxTessEvaluationImageUniforms = 0;
	Resources.maxGeometryImageUniforms = 0;
	Resources.maxFragmentImageUniforms = 8;
	Resources.maxCombinedImageUniforms = 8;
	Resources.maxGeometryTextureImageUnits = 16;
	Resources.maxGeometryOutputVertices = 256;
	Resources.maxGeometryTotalOutputComponents = 1024;
	Resources.maxGeometryUniformComponents = 1024;
	Resources.maxGeometryVaryingComponents = 64;
	Resources.maxTessControlInputComponents = 128;
	Resources.maxTessControlOutputComponents = 128;
	Resources.maxTessControlTextureImageUnits = 16;
	Resources.maxTessControlUniformComponents = 1024;
	Resources.maxTessControlTotalOutputComponents = 4096;
	Resources.maxTessEvaluationInputComponents = 128;
	Resources.maxTessEvaluationOutputComponents = 128;
	Resources.maxTessEvaluationTextureImageUnits = 16;
	Resources.maxTessEvaluationUniformComponents = 1024;
	Resources.maxTessPatchComponents = 120;
	Resources.maxPatchVertices = 32;
	Resources.maxTessGenLevel = 64;
	Resources.maxViewports = 16;
	Resources.maxVertexAtomicCounters = 0;
	Resources.maxTessControlAtomicCounters = 0;
	Resources.maxTessEvaluationAtomicCounters = 0;
	Resources.maxGeometryAtomicCounters = 0;
	Resources.maxFragmentAtomicCounters = 8;
	Resources.maxCombinedAtomicCounters = 8;
	Resources.maxAtomicCounterBindings = 1;
	Resources.maxVertexAtomicCounterBuffers = 0;
	Resources.maxTessControlAtomicCounterBuffers = 0;
	Resources.maxTessEvaluationAtomicCounterBuffers = 0;
	Resources.maxGeometryAtomicCounterBuffers = 0;
	Resources.maxFragmentAtomicCounterBuffers = 1;
	Resources.maxCombinedAtomicCounterBuffers = 1;
	Resources.maxAtomicCounterBufferSize = 16384;
	Resources.maxTransformFeedbackBuffers = 4;
	Resources.maxTransformFeedbackInterleavedComponents = 64;
	Resources.maxCullDistances = 8;
	Resources.maxCombinedClipAndCullDistances = 8;
	Resources.maxSamples = 4;
	Resources.limits.nonInductiveForLoops = 1;
	Resources.limits.whileLoops = 1;
	Resources.limits.doWhileLoops = 1;
	Resources.limits.generalUniformIndexing = 1;
	Resources.limits.generalAttributeMatrixVectorIndexing = 1;
	Resources.limits.generalVaryingIndexing = 1;
	Resources.limits.generalSamplerIndexing = 1;
	Resources.limits.generalVariableIndexing = 1;
	Resources.limits.generalConstantMatrixVectorIndexing = 1;
}

EShLanguage FindLanguage(const vk::ShaderStageFlagBits shaderType)
{
	switch (shaderType)
	{
	case vk::ShaderStageFlagBits::eVertex:
		return EShLangVertex;

	case vk::ShaderStageFlagBits::eTessellationControl:
		return EShLangTessControl;

	case vk::ShaderStageFlagBits::eTessellationEvaluation:
		return EShLangTessEvaluation;

	case vk::ShaderStageFlagBits::eGeometry:
		return EShLangGeometry;

	case vk::ShaderStageFlagBits::eFragment:
		return EShLangFragment;

	case vk::ShaderStageFlagBits::eCompute:
		return EShLangCompute;

	default:
		return EShLangVertex;
	}
}
}
#endif

void VulkanRenderer::Utils::InitShaderCompiler()
{
#ifndef __ANDROID__
	glslang::InitializeProcess();
#endif
}

std::vector<unsigned int> VulkanRenderer::Utils::CompileShader(
	const vk::ShaderStageFlagBits shaderType, const std::string& shaderSource)
{
	std::vector<unsigned int> spirv;

#ifndef __ANDROID__
	const EShLanguage stage = FindLanguage(shaderType);
	glslang::TShader shader(stage);
	TBuiltInResource resources;
	InitResources(resources);

	// Enable SPIR-V and Vulkan rules when parsing GLSL
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

	const char* const shaderStrings[] = { shaderSource.data() };
	shader.setStrings(shaderStrings, 1);

	if (!shader.parse(&resources, 100, false, messages))
	{
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		throw std::runtime_error("Shader parsing error.");
	}

	glslang::TProgram program;
	program.addShader(&shader);

	//
	// Program-level processing...
	//

	if (!program.link(messages))
	{
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		fflush(stdout);
		throw std::runtime_error("Shader compilation error.");
	}

	glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);
#else
	// On Android, use shaderc instead.
	shaderc::Compiler compiler;
	shaderc::SpvCompilationResult module =
		compiler.CompileGlslToSpv(pshader, strlen(pshader),
			MapShadercType(shader_type),
			"shader");
	if (module.GetCompilationStatus() !=
		shaderc_compilation_status_success)
	{
		LOGE("Error: Id=%d, Msg=%s",
			module.GetCompilationStatus(),
			module.GetErrorMessage().c_str());
		return false;
	}
	spirv.assign(module.cbegin(), module.cend());
#endif

	return spirv;
}

void VulkanRenderer::Utils::FinalizeShaderCompiler()
{
#ifndef __ANDROID__
	glslang::FinalizeProcess();
#endif
}
