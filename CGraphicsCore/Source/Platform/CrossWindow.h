#pragma once

#include "CGR/Core/Core.h"
#include "CGR/Core/Window.h"
#include "CGR/Renderer/RendererContext.h"

#include "GLFW/glfw3.h"

namespace Cgr
{
	// Platform agnostic window class
	class CrossWindow : public Window
	{
	public:
		CrossWindow(const WindowProps& props);
		~CrossWindow();

		void Init(const WindowProps& props);

		virtual void OnUpdate() override;

		virtual void* GetNativeWindow() const override { return m_WindowHandle; }

		virtual uint32_t GetWidth() const override { return m_WindowData.Width; }
		virtual uint32_t GetHeight() const override { return m_WindowData.Height; }

		virtual void SetEventCallback(const EventCallbackFn& callback) override { m_WindowData.EventCallback = callback; }

		virtual void SetVSync(bool enabled) override;
		virtual bool IsVSync() const override;

		void Shutdown();

	private:
		GLFWwindow* m_WindowHandle;
		RendererContext* m_RendererContext;
		struct WindowData
		{
			std::string Title;
			uint32_t Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_WindowData;
	};
}