#include "CGRpch.h"
#include "OpenGLVertexArray.h"
#include "OpenGLShader.h"

#include <glad/glad.h>

namespace Cgr
{
	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &m_RendererID);
		glBindVertexArray(m_RendererID);
		m_IsBinded = true;
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind()
	{
		glBindVertexArray(m_RendererID);
		m_IsBinded = true;
	}

	void OpenGLVertexArray::Unbind()
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::SetBufferLayout(const BufferLayout& bufferLayout)
	{
		m_BufferLayout = bufferLayout;
		glBindVertexArray(m_RendererID);

		auto& elements = m_BufferLayout.GetBufferElements();

		for (auto& e : elements)
		{
			glVertexAttribPointer(e.AttribIndex, GetElementCount(e.Type),
				OpenGLShader::ToOpenGLDataType(e.Type), e.IsNormalized,
				m_BufferLayout.GetStride(), reinterpret_cast<const void*>(static_cast<std::uintptr_t>(e.Offset)));
			glEnableVertexAttribArray(e.AttribIndex);
		}
	}
}
