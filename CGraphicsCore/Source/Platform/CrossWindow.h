#pragma once

#include "CGR/Core/Core.h"
#include "CGR/Core/Window.h"
#include "CGR/Renderer/RendererContext.h"

#include "GLFW/glfw3.h"

namespace Cgr
{
	class CrossWindow : public Window
	{
	public:
		CrossWindow(const WindowProps& props);
		~CrossWindow();

		void Init(const WindowProps& props);

		virtual void OnUpdate() override;

		virtual void* GetNativeWindow() const override { return m_WindowHandle; }

		virtual uint32_t GetWidth() const override { return m_WindowProps.Width; }
		virtual uint32_t GetHeight() const override { return m_WindowProps.Height; }

		virtual void SetVSync(bool enabled) override;
		virtual bool IsVSync() const override;

		void Shutdown();

	private:
		GLFWwindow* m_WindowHandle;
		RendererContext* m_RendererContext;
		WindowProps m_WindowProps;
	};
}