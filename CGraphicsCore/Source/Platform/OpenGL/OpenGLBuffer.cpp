#include "CGRpch.h"
#include "OpenGLBuffer.h"
#include "CGR/Renderer/RendererAPI.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Cgr
{
	static uint32_t s_BindingPoint = 1;

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

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &m_RendererID);
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
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t count, const uint32_t* indices, BufferDrawUsage usage)
		: m_Count(count)
	{
		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, BufferDrawUsageToOpenGLDrawUsage(usage));
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLIndexBuffer::Bind()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}
	void OpenGLIndexBuffer::Unbind()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void OpenGLIndexBuffer::SetData(int64_t offset, uint32_t count, const uint32_t* indices)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, count * sizeof(uint32_t), indices);
	}

	OpenGLUniformBuffer::OpenGLUniformBuffer(const std::string& blockName)
		: m_BlockName(blockName)
	{
		m_BindingPoint = s_BindingPoint;
		s_BindingPoint++;

		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
		glBufferData(GL_UNIFORM_BUFFER, 512, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_RendererID);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLUniformBuffer::Bind()
	{
		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
		glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_RendererID);
	}

	void OpenGLUniformBuffer::Unbind()
	{
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void OpenGLUniformBuffer::SetBlockBinding(uint32_t shaderID)
	{
		uint32_t blockIndex = glGetUniformBlockIndex(shaderID, m_BlockName.c_str());
		glUniformBlockBinding(shaderID, blockIndex, m_BindingPoint);
	}

	void OpenGLUniformBuffer::SetData(int64_t offset, int64_t size, const void* data)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
		glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_RendererID);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	}

	OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer()
	{
		int64_t bufferSize = static_cast<int64_t>(2 * 1024) * 1024; // 2 MB;

		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
		/*glBufferStorage(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr,
			GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);*/

		glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_RendererID);

		//m_DataPtr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, bufferSize,
		//	GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

		//if (m_DataPtr == nullptr)
		//{
		//	CGR_CORE_ERROR("Failed to map buffer persistently!");
		//}
	}

	OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer()
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLShaderStorageBuffer::Bind()
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
	}

	void OpenGLShaderStorageBuffer::Unbind()
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	
	void OpenGLShaderStorageBuffer::SetData(int64_t offset, int64_t size, const void* data)
	{
		//memcpy(reinterpret_cast<char*>(m_DataPtr) + offset, data, size);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
	}
}
