#pragma once

#include "Core.h"
#include "Window.h"
#include "CGR/Asset/AssetManager.h"
#include "CGR/Event/MouseEvent.h"
#include "CGR/UI/ImGuiLayer.h"

namespace Cgr
{
	class WindowClosedEvent;

	class CGR_API Application
	{
	public:
		Application(std::string title, uint32_t width, uint32_t height);
		~Application();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		void OnEvent(Event& e);
		void Run();
		void Close();

		const Window& GetWindow() const { return *m_Window; }
		ImGuiLayer* GetUILayer() { return m_ImGuiLayer; }
		AssetManager* GetAssetManager() { return m_AssetManager; }

		static Application& Get();
		void OnWindowClose(WindowClosedEvent& e);

	private:
		bool m_IsRunning = false;
		bool m_IsMinimized= false;

		Scope<Window> m_Window;

	private:
		static Application* s_Application;
		AssetManager* m_AssetManager;

		ImGuiLayer* m_ImGuiLayer;

		LayerStack m_LayerStack;
		EventManager m_EventManager;
		
		float m_LastFrameTime = 0.0f;
	};


	Application* CreateApplication();
}