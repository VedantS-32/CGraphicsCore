#include "CGRpch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Cgr
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		CGR_CORE_ASSERT(windowHandle, "Window handle is null");
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		CGR_CORE_ASSERT(status, "Failed to initialize Glad");

		CGR_CORE_INFO("OpenGL Info:");
		CGR_CORE_INFO("    Vendor: {0}", ((char*)glGetString(GL_VENDOR)));
		CGR_CORE_INFO("    Renderer: {0}", ((char*)glGetString(GL_RENDERER)));
		CGR_CORE_INFO("    Version: {0}", ((char*)glGetString(GL_VERSION)));
	}

	void OpenGLContext::SwapBuffer()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
}
