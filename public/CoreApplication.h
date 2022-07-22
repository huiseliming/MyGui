#pragma once
#include "MyCppForward.h"
#include <spdlog/spdlog.h>

class MYCPP_API CoreApplication
{
public:
	CoreApplication() = default;

	virtual void Run(int& ArgC, const char* ArgV[]);
	virtual bool Startup(int& ArgC, const char* ArgV[]);
	virtual void MainLoop();
	virtual void Cleanup();
	virtual bool IsRequestExit();
	virtual void SetRequestExit();

	ThreadTaskQueue* MainThreadTaskQueue() { return _MainThreadTaskQueue; }
	const std::vector<std::string>& CommandLineArgsRef() { return _CommandLineArgs; }

	// 发送一个任务到主线程
	template<typename Function, typename ... FunctionArgs>
	bool PostTaskToMainThread(Function&& task, FunctionArgs&& ... args)
	{
		if (_MainThreadTaskQueue)
		{
			_MainThreadTaskQueue->EnqueueTask(std::bind(std::forward<Function>(task), std::forward<FunctionArgs>(task)...));
			return true;
		}
		else
		{
			spdlog::error("CoreApplication::_MainThreadTaskQueue IS NULLPTR");
		}
		return false;
	}

protected:
	ThreadTaskQueue* _MainThreadTaskQueue{ nullptr };
	std::vector<std::string> _CommandLineArgs;
	bool bRequestExit = false;
};

MYCPP_API extern CoreApplication* GCoreApplication;
