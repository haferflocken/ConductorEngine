#pragma once

#include <vulkanrenderer/vulkanobjects/InfoObjects.h>
#include <vulkanrenderer/VulkanPlatform.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#define ASSERT_SUCCESS(result, errorMessage) if (result != vk::Result::eSuccess) throw std::runtime_error(errorMessage);

namespace VulkanRenderer
{
namespace VulkanObjects
{
class WindowObject
{
public:
	WindowObject(const vk::Instance& instanceRef, const ApplicationInfo& applicationInfo, const WindowInfo& windowInfo)
		: m_instanceRef(instanceRef)
		, m_window(nullptr)
	{
		// Create an SDL window which will present a Vulkan surface.
		m_window = SDL_CreateWindow(applicationInfo.name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			windowInfo.width, windowInfo.height, SDL_WINDOW_OPENGL);
		if (m_window == nullptr)
		{
			throw std::runtime_error("Failed to create SDL window.");
		}

		SDL_SysWMinfo sdlWindowInfo;
		SDL_VERSION(&sdlWindowInfo.version);
		if (!SDL_GetWindowWMInfo(m_window, &sdlWindowInfo))
		{
			throw std::runtime_error("Failed to get SDL window info.");
		}

		switch (sdlWindowInfo.subsystem)
		{
#if defined(SDL_VIDEO_DRIVER_ANDROID) && defined(VK_USE_PLATFORM_ANDROID_KHR)
		case SDL_SYSWM_ANDROID:
		{
			vk::AndroidSurfaceCreateInfoKHR surfaceInfo = vk::AndroidSurfaceCreateInfoKHR()
				.setWindow(sdlWindowInfo.info.android.window);
			const vk::Result result = instance.createAndroidSurfaceKHR(surfaceInfo);
			ASSERT_SUCCESS(result, "Failed to create window surface.");
			break;
		}
#endif

#if defined(SDL_VIDEO_DRIVER_MIR) && defined(VK_USE_PLATFORM_MIR_KHR)
		case SDL_SYSWM_MIR:
		{
			vk::MirSurfaceCreateInfoKHR surfaceInfo = vk::MirSurfaceCreateInfoKHR()
				.setConnection(sdlWindowInfo.info.mir.connection)
				.setMirSurface(sdlWindowInfo.info.mir.surface);
			const vk::Result result = instance.createMirSurfaceKHR(surfaceInfo);
			ASSERT_SUCCESS(result, "Failed to create window surface.");
			break;
		}
#endif

#if defined(SDL_VIDEO_DRIVER_WAYLAND) && defined(VK_USE_PLATFORM_WAYLAND_KHR)
		case SDL_SYSWM_WAYLAND:
		{
			vk::WaylandSurfaceCreateInfoKHR surfaceInfo = vk::WaylandSurfaceCreateInfoKHR()
				.setDisplay(sdlWindowInfo.info.wl.display)
				.setSurface(sdlWindowInfo.info.wl.surface);
			const vk::Result result = instance.createWaylandSurfaceKHR(surfaceInfo);
			ASSERT_SUCCESS(result, "Failed to create window surface.");
			break;
		}
#endif

#if defined(SDL_VIDEO_DRIVER_WINDOWS) && defined(VK_USE_PLATFORM_WIN32_KHR)
		case SDL_SYSWM_WINDOWS:
		{
			const vk::Win32SurfaceCreateInfoKHR surfaceInfo = vk::Win32SurfaceCreateInfoKHR()
				.setHinstance(GetModuleHandle(nullptr))
				.setHwnd(sdlWindowInfo.info.win.window);
			const vk::Result result = m_instanceRef.createWin32SurfaceKHR(&surfaceInfo, nullptr, &m_surface);
			ASSERT_SUCCESS(result, "Failed to create window surface.");
			break;
		}
#endif

#if defined(SDL_VIDEO_DRIVER_X11) && defined(VK_USE_PLATFORM_XLIB_KHR)
		case SDL_SYSWM_X11:
		{
			vk::XlibSurfaceCreateInfoKHR surfaceInfo = vk::XlibSurfaceCreateInfoKHR()
				.setDpy(sdlWindowInfo.info.x11.display)
				.setWindow(sdlWindowInfo.info.x11.window);
			const vk::Result result = instance.createXlibSurfaceKHR(surfaceInfo);
			ASSERT_SUCCESS(result, "Failed to create window surface.");
			break;
		}
#endif
		default:
		{
			throw std::runtime_error("Unknown SDL window subsystem.");
		}
		}
	}

	~WindowObject()
	{
		m_instanceRef.destroySurfaceKHR(m_surface);
		SDL_DestroyWindow(m_window);
		SDL_Quit();
	}

	const vk::SurfaceKHR& GetSurface() const { return m_surface; }

private:
	const vk::Instance& m_instanceRef;

	vk::SurfaceKHR m_surface;
	SDL_Window* m_window;
};
}
}
