#include "CGRpch.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>

namespace Cgr
{
	// For wrong type it will return GL_STATIC_DRAW as default
	static GLenum BufferDrawUsageToOpenGLDrawUsage(BufferDrawUsage usage)
	{
		switch (usage)
		{
		case Cgr::BufferDrawUsage::StaticDraw:
			return GL_STATIC_DRAW;
			break;
		case Cgr::BufferDrawUsage::StreamDraw:
			return GL_STREAM_DRAW;
			break;
		case Cgr::BufferDrawUsage::DynamicDraw:
			return GL_DYNAMIC_DRAW;
			break;
		default:
			break;
		}

		return GL_STATIC_DRAW;
	}

	// Vertex Buffer
	OpenGLVertexBuffer::OpenGLVertexBuffer(int64_t size, const void* data, BufferDrawUsage usage)
	{
		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, data, BufferDrawUsageToOpenGLDrawUsage(usage));

	}

	void OpenGLVertexBuffer::Bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::Unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLVertexBuffer::SetData(int64_t offset, int64_t size, const void* data)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
	}

	// Index Buffer
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t count, uint32_t* indices, BufferDrawUsage usage)
		: m_Count(count)
	{
		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, BufferDrawUsageToOpenGLDrawUsage(usage));
	}

	void OpenGLIndexBuffer::Bind()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}
	void OpenGLIndexBuffer::Unbind()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void OpenGLIndexBuffer::SetData(int64_t offset, uint32_t count, uint32_t* indices)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, count * sizeof(uint32_t), indices);
	}
}
