#include "CoreApplication.h"

CoreApplication* GP_CoreApplication;

void CoreApplication::Run(int& ArgC, const char* ArgV[])
{
	if (Startup(ArgC, ArgV))
	{
		while (!IsRequestExit()) {
			MainLoop();
		}
	}
	Cleanup();
}

bool CoreApplication::Startup(int& ArgC, const char* ArgV[])
{
	GP_CoreApplication = this;
	_MainThreadTaskQueue = &CurrentThreadTaskQueueRef();
	return true;
}

void CoreApplication::MainLoop()
{
	MainThreadTaskQueue()->ProcessTask();
	std::this_thread::sleep_for(std::chrono::microseconds(1));
}

void CoreApplication::Cleanup()
{
}

bool CoreApplication::IsRequestExit()
{
	return bRequestExit;
}

void CoreApplication::SetRequestExit()
{
	bRequestExit = true;
}

