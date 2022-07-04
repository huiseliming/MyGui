#pragma once
#include "MyGuiForward.h"
#include "CoreApplication.h"
#include "GuiDrawObject.h"
#include <spdlog/spdlog.h> 
#include <Poco/Util/Application.h>
#include <Poco/Util/HelpFormatter.h>

#define IMGUI_API MYGUI_API
#define IMGUI_IMPL_API IMGUI_API
#include "../third_party/imgui/imgui.h"
#include "../third_party/imgui/misc/cpp/imgui_stdlib.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

constexpr static int MaxFramesInFlight = 2;

static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

MYGUI_API VkSurfaceFormatKHR ImGui_ImplVulkanH_SelectSurfaceFormat_HookFunction(VkPhysicalDevice physical_device, VkSurfaceKHR surface, const VkFormat* request_formats, int request_formats_count, VkColorSpaceKHR request_color_space);

class MYGUI_API TaskQueueSubsystem : public Poco::Util::Subsystem
{
	using Super = Poco::Util::Subsystem;
public:

	uint32_t ProcessMainThreadTasks()
	{
		return _MainThreadTaskQueue->ProcessTask();
	}

	virtual const char* name() const override
	{
		return "TaskQueueSubsystem";
	}

protected:
	virtual void initialize(Poco::Util::Application& app) override
	{
		_MainThreadTaskQueue = &CurrentThreadTaskQueueRef();
	}

	virtual void uninitialize() override
	{
		ProcessMainThreadTasks();
		_MainThreadTaskQueue = nullptr;
	}

	template<typename Task, typename ... TaskArgs>
	bool PostTaskToMainThread(Task&& task, TaskArgs&& ... args)
	{
		if (_MainThreadTaskQueue)
		{
			_MainThreadTaskQueue->EnqueueTask(std::bind(std::forward<Task>(task), std::forward<TaskArgs>(task)...));
			return true;
		}
		else
		{
			spdlog::error("TaskQueueSubsystem::_MainThreadTaskQueue IS NULLPTR");
		}
		return false;
	}

	ThreadTaskQueue* _MainThreadTaskQueue{ nullptr };
};



class MYGUI_API VulkanGuiApplication : public Poco::Util::Application
{
	using Super = Poco::Util::Application;
public:
	VulkanGuiApplication();
protected:
	virtual void initialize(Poco::Util::Application& self) override;
	virtual void reinitialize(Poco::Util::Application& self) override;
	virtual void uninitialize() override;
	virtual void defineOptions(Poco::Util::OptionSet& options) override;
	virtual void handleOption(const std::string& name, const std::string& value) override;

	void HandleHelp(const std::string& name, const std::string& value) {
		_SkipMain = true;
		Poco::Util::HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("A sample application that demonstrates some of the features of the Poco::Util::Application class.");
		helpFormatter.format(std::cout);
		stopOptionsProcessing();
	}

	virtual int main(const std::vector<std::string>& args) override
	{
		int exit_code = EXIT_SUCCESS;
		try
		{
			std::vector<const char*> startup_args;
			for (size_t i = 0; i < args.size(); i++)
			{
				startup_args.push_back(args[i].c_str());
			}
			if (Startup(startup_args.size(), startup_args.data()))
			{
				while (!IsRequestExit()) {
					MainLoop();
				}
			}
		}
		catch (const Poco::Exception& PocoException)
		{
			exit_code = EXIT_FAILURE;
			spdlog::error("[std::exception] <what:{:s}>", PocoException.message());
		}
		catch (const std::exception& StdException)
		{
			exit_code = EXIT_FAILURE;
			spdlog::error("[std::exception] <what:{:s}>", StdException.what());
		}
		Cleanup();
		return exit_code;
	}

	GLFWwindow* GLFWwindowPtr() { return _GLFWwindow; }

private:
	bool _SkipMain = false;
	Poco::AutoPtr<TaskQueueSubsystem> _TaskQueueSubsystem;
protected:

	bool Startup(int32_t ArgC, const char* ArgV[]);
	void MainLoop();
	void Cleanup();
	bool IsRequestExit();
	void SetRequestExit();

	uint32_t _Width = 1280;
	uint32_t _Height = 720;

	// GLFW
	GLFWwindow* _GLFWwindow{ nullptr };

	// ImGui
	void ImGui_CreateDescriptorPool();
	void ImGui_CreateCommandPoolAndAllocateCommandBuffer();
	void ImGui_CreateRenderPass();
	void ImGui_CreateFramebuffer();
	void ImGui_CleanupSwapChain();
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
	void ImGui_BeginDockSpace();
	void ImGui_DrawWidget();
	void ImGui_EndDockSpace();
public:
	std::map<std::string, std::function<void()>> _ImGuiDrawFunctions;

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
public:
	// 
	vk::SurfaceFormatKHR& SurfaceFormatRef() { return _SurfaceFormat; }

private:
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
























