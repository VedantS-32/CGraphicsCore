#include "CGRpch.h"
#include "OpenGLAPI.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Cgr
{
	API RendererAPI::m_API = API::OpenGL;

#ifdef CGR_DEBUG

	static void GLAPIENTRY
		MessageCallback(GLenum source,
			GLenum type,
			GLuint id,
			GLenum severity,
			GLsizei length,
			const GLchar* message,
			const void* userParam)
	{
		type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "";
		CGR_CORE_TRACE("GL CALLBACK: type = 0x{0}, severity = 0x{1}, message = {2}", type, severity, message);
	}

	static void SetGLDebugMessageCallback()
	{
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0);
	}

#else
	static void SetGLDebugMessageCallback() {}

#endif // CGR_DEBUG

	void OpenGLAPI::Init()
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		SetGLDebugMessageCallback();
	}

	void OpenGLAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}
}
