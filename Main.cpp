#pragma once
#include "VulkanGuiApplication.h"


int main(int ArgC, const char* ArgV[])
{
	class D : public ImGuiDrawable
	{
		virtual void Draw()
		{
			ImGui::ShowDemoWindow();
		}

	};
	VulkanGuiApplication app;
	app.Drawables.push_back(std::make_shared<D>());
	app.Run(ArgC, ArgV);
}


