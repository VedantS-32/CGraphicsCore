#pragma once

#include "Core.h"
#include "Window.h"

namespace Cgr
{
	class CGR_API Application
	{
	public:
		Application(std::string title, uint32_t width, uint32_t height);

		void Run();

		void Close();

		const Window& GetWindow() const { return *m_Window; }

		static Application& Get();

	private:
		bool m_IsRunning = false;
		bool m_IsMinimized= false;

		Scope<Window> m_Window;

	private:
		static Application* s_Application;
	};

	Application* CreateApplication();
}