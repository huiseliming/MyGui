#include "CoreApplication.h"

CoreApplication* GP_CoreApplication;

void CoreApplication::Run(int& ArgC, const char* ArgV[])
{
	GP_CoreApplication = this;
	if (Startup(ArgC, ArgV))
	{
		MainLoop();
	}
	Cleanup();
}

bool CoreApplication::Startup(int& ArgC, const char* ArgV[])
{
	return true;
}

void CoreApplication::MainLoop()
{
}

void CoreApplication::Cleanup()
{
}
