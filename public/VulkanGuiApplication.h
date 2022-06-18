#pragma once
#include "EasyGuiForward.h"
#include "ImGuiDrawable.h"
#include <spdlog/spdlog.h>

#define IMGUI_API EASYGUI_API
#define IMGUI_IMPL_API IMGUI_API
#include "../third_party/imgui/imgui.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

constexpr static int MaxFramesInFlight = 2;

static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

class EASYGUI_API VulkanGuiApplication
{
public:
	void Run(int32_t ArgC, const char* ArgV[]) {
		try
		{
			if (Startup(ArgC, ArgV))
			{
				MainLoop();
			}
		}
		catch (const std::exception& StdException)
		{
			spdlog::error("[std::exception] <what:{:s}>", StdException.what());
		}
		Cleanup();
	}

private:

	bool Startup(int32_t ArgC, const char* ArgV[]);
	void MainLoop();
	void Cleanup();

	uint32_t _Width = 1280;
	uint32_t _Height = 720;

	// GLFW
	GLFWwindow* _GLFWwindow{ nullptr };

	// ImGui
	void ImGui_CreateDescriptorPool();
	void ImGui_CreateCommandPoolAndAllocateCommandBuffer();
	void ImGui_CreateRenderPass();
	void ImGui_CreateFramebuffer();
	void ImGui_RecordCommand(vk::CommandBuffer& command_buffer, uint32_t image_index);
	void ImGui_Startup();
	void ImGui_RenderFrame();
	void ImGui_Cleanup();

	vk::RenderPass _ImGuiRenderPass;
	vk::DescriptorPool _ImGuiDescriptorPool;
	vk::CommandPool _ImGuiCommandPool;
	std::vector<vk::CommandBuffer> _ImGuiCommandBuffers;
	std::vector<vk::Framebuffer> _ImGuiFramebuffers;

	// Draw logic
	void ImGui_Draw();
public:
	std::vector<std::shared_ptr<ImGuiDrawable>> Drawables;
private:
	// Tools
	bool CheckValidationLayerSupport();
	bool FindQueueFamily(std::vector<vk::QueueFamilyProperties, std::allocator<vk::QueueFamilyProperties>> queue_family_properties);
	void ExecuteGraphicsCommandAndWaitComplete(std::function<void(vk::CommandBuffer&)> record_command_callback);
	void ExecuteGraphicsCommand(std::function<void(vk::CommandBuffer&)> record_command_callback, std::function<void()> complete_callback)
	{
	}

	// Vulkan 
	void CreateInstance();
	void CreateSurface();
	void CreateDevice();
	void CreateSwapchain();
	void CreateSyncObjects();
	void DestroySyncObjects();
	void CreateRenderPass();
	void CreateFramebuffers();
	void CreateCommandPoolAndAllocateCommandBuffer();
	void CleanupSwapChain();
	void RecreateSwapchain();
	void RecordCommand(vk::CommandBuffer& command_buffer, uint32_t image_index);
	void DrawFrame();


	const std::vector<const char*> _ValidationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	const bool _EnableValidationLayers = false;
#else
	const bool _EnableValidationLayers = true;
#endif
	vk::Instance _Instance;
	vk::SurfaceKHR _Surface;
	//Deivce
	vk::PhysicalDevice _PhysicalDevice;
	uint32_t _GraphicsQueueFamily;
	uint32_t _PresentQueueFamily;
	vk::PhysicalDeviceFeatures _PhysicalDeviceFeatures;
	vk::Device _Device;
	vk::Queue _GraphicsQueue;
	vk::Queue _PresentQueue;
	//Swapchain
	vk::SurfaceFormatKHR _SurfaceFormat;
	vk::PresentModeKHR _SurfacePresentMode;
	vk::Extent2D _SurfaceExtent;
	uint32_t _MinImageCount;
	uint32_t _ImageCount;
	vk::SwapchainKHR _Swapchain;
	std::vector<vk::Image> _SwapchainImages;
	std::vector<vk::ImageView> _SwapchainImageViews;

	// Render Object
	vk::RenderPass _RenderPass;
	std::vector<vk::Framebuffer> _SwapchainFramebuffers;
	vk::CommandPool _CommandPool;
	std::vector<vk::CommandBuffer> _CommandBuffers;


	// 
	std::vector<vk::Semaphore> _ImageAvailableSemaphores;
	std::vector<vk::Semaphore> _RenderFinishedSemaphores;
	std::vector<vk::Fence> _InFlightFences;
	uint32_t _CurrentFrame = 0;


	bool _FramebufferResized = false;
	friend void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
};

static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto vulkan_gui_application = reinterpret_cast<VulkanGuiApplication*>(glfwGetWindowUserPointer(window));
	vulkan_gui_application->_FramebufferResized = true;
}
























