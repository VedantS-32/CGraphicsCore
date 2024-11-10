#pragma once

#include "CGR/Core/Core.h"

namespace Cgr
{
	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;
		bool VSync;

		WindowProps(std::string title = "CGraphics Sandbox", uint32_t width = 1080, uint32_t height = 720, bool vsync = true) :
			Title(title),
			Width(width),
			Height(height),
			VSync(vsync)
		{}
	};

	class CGR_API Window
	{
	public:
		virtual ~Window() {};

		virtual void OnUpdate() = 0;

		virtual void* GetNativeWindow() const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		static Scope<Window> Create(const WindowProps& props = WindowProps());
	};
}