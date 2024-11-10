#pragma once

#include "CGR/Renderer/RendererContext.h"

struct GLFWwindow;

namespace Cgr
{
	class OpenGLContext : public RendererContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffer() override;

	private:
		GLFWwindow* m_WindowHandle;
	};
}