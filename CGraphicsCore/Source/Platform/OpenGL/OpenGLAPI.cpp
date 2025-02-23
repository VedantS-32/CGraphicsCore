#include "CGRpch.h"
#include "OpenGLAPI.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Cgr
{
	API RendererAPI::m_API = API::OpenGL;

#ifndef CGR_DEBUG

	static void GLAPIENTRY
		MessageCallback(GLenum source,
			GLenum type,
			GLuint id,
			GLenum severity,
			GLsizei length,
			const GLchar* message,
			const void* userParam)
	{
		std::string errorType;
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:               errorType = "ERROR"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: errorType = "DEPRECATED"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  errorType = "UNDEFINED_BEHAVIOR"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         errorType = "PORTABILITY"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         errorType = "PERFORMANCE"; break;
		case GL_DEBUG_TYPE_MARKER:              errorType = "MARKER"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          errorType = "PUSH_GROUP"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           errorType = "POP_GROUP"; break;
		case GL_DEBUG_TYPE_OTHER:               errorType = "OTHER"; break;
		default: errorType = "UNKNOWN"; break;
		}

		if ( severity != GL_DEBUG_SEVERITY_LOW && severity != GL_DEBUG_SEVERITY_NOTIFICATION)
		{
			type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "";
			switch (severity)
			{
			case GL_DEBUG_SEVERITY_MEDIUM:
				CGR_CORE_WARN("GL CALLBACK: type = {0}, severity = MEDIUM, message = {1}", errorType, message);
				break;
			case GL_DEBUG_SEVERITY_HIGH:
				CGR_CORE_ERROR("GL CALLBACK: type = {0}, severity = HIGH, message = {1}", errorType, message);
				break;
			default:
				break;
			}
		}
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

	void OpenGLAPI::DrawIndexed(uint32_t indexCount)
	{
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLAPI::EnableDepthMask(bool enable)
	{
		if (enable)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);
	}

	void OpenGLAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}
}
