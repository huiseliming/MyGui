//#pragma once
//#include "EasyGuiForward.h"
//#include "VkException.h"
//#include <spdlog/spdlog.h>
//#include "ImGuiImpl.h"
//
//class VulkanGraphics;
//
//static void GLFWErrorCallback(int error, const char* description)
//{
//	spdlog::error("Glfw Error {0:d}: {1:s}\n", error, description);
//}
//
//static void CheckVkResult(VkResult vk_result)
//{
//	if (vk_result != VK_SUCCESS) [[unlikely]] {
//		//spdlog::error("[] ASSERT  <VkResult:{:s}({:d})>", VkResultToString(vk_result), vk_result);
//	}
//}
//class Application
//{
//public:
//	Application() = default;
//	void Run(int& ArgC, const char* ArgV[])
//	{
//		Initialize(ArgC, ArgV);
//		MainLoop();
//		Deinitialize();
//	}
//
//	bool Initialize(int& ArgC, const char* ArgV[]);
//
//	void MainLoop();
//
//	void Deinitialize();
//
//	ThreadTaskQueue* MainThreadTaskQueue() { return _MainThreadTaskQueue; }
//	const std::vector<std::string>& CommandLineArgsRef() { return _CommandLineArgs; }
//
//	// 发送一个任务到主线程
//	template<typename Function, typename ... FunctionArgs>
//	bool PostTaskToMainThread(Function&& task, FunctionArgs&& ... args)
//	{
//		if (_MainThreadTaskQueue)
//		{
//			_MainThreadTaskQueue->EnqueueTask(std::bind(std::forward<Function>(task), std::forward<FunctionArgs>(task)...));
//			return true;
//		}
//		return false;
//	}
//
//	GLFWwindow* GLFWwindowPtr() { return _GLFWwindow; }
//
//private:
//	GLFWwindow* _GLFWwindow;
//	ThreadTaskQueue* _MainThreadTaskQueue{ nullptr };
//	std::vector<std::string> _CommandLineArgs;
//	std::unique_ptr<VulkanGraphics> _VulkanGraphics;
//};
//
//EASYGUI_API extern Application* GP_Application;
//EASYGUI_API extern std::shared_ptr<spdlog::logger> GSP_Logger;
