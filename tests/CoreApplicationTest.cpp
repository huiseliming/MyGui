#pragma once
#include "VulkanGuiApplication.h"


int main(int ArgC, const char* ArgV[])
{
	CoreApplication core_application;
	std::thread Thread([&] {
		while (!core_application.PostTaskToMainThread(
			[&] { 
				core_application.SetRequestExit(); 
			}));
		});
	core_application.Run(ArgC, ArgV);
	Thread.join();
}


