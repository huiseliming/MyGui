//#include "EasyApplication.h"
//#include "Graphics/VulkanGraphics.h"
//
//Application* GApplication = nullptr;
//
//inline bool Application::Initialize(int& ArgC, const char* ArgV[])
//{
//	for (size_t i = 0; i < ArgC; i++)
//	{
//		_CommandLineArgs.push_back(ArgV[i]);
//	}
//
//	// Setup GLFW window
//	glfwSetErrorCallback(GLFWErrorCallback);
//	if (!glfwInit())
//		return 1;
//
//	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//	_GLFWwindow = glfwCreateWindow(1280, 720, "EasyGui", NULL, NULL);
//
//	// Setup Vulkan
//	if (!glfwVulkanSupported())
//	{
//		spdlog::error("GLFW: Vulkan Not Supported");
//		return false;
//	}
//
//	_VulkanGraphics = std::make_unique<VulkanGraphics>();
//	_VulkanGraphics->CreateInstance();
//
//	VkSurfaceKHR surface;
//	VK_ASSERT_SUCCESSED(glfwCreateWindowSurface(_VulkanGraphics->_Instance, _GLFWwindow, nullptr, &surface));
//	_VulkanGraphics->CreateDevice(surface);
//
//	int width, height;
//	glfwGetFramebufferSize(_GLFWwindow, &width, &height);
//	_VulkanGraphics->CreateSwapchain(width, height);
//
//	_VulkanGraphics->CreateCommands();
//	_VulkanGraphics->CreateRenderPass();
//	_VulkanGraphics->CreateFrameBuffers();
//	_VulkanGraphics->CreateSyncObject();
//
//	return true;
//}
//
//inline void Application::MainLoop()
//{
//	while (!glfwWindowShouldClose(_GLFWwindow))
//	{
//		glfwPollEvents();
//		_VulkanGraphics->Draw();
//	}
//}
//
//inline void Application::Deinitialize()
//{
//	VK_ASSERT_SUCCESSED(vkDeviceWaitIdle(_VulkanGraphics->_Device));
//
//	_VulkanGraphics.reset();
//
//	glfwDestroyWindow(_GLFWwindow);
//	glfwTerminate();
//}
