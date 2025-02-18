#pragma once

#include "CGR/Renderer/VertexArray.h"
#include "CGR/Renderer/Buffer.h"

#include <vector>

namespace Cgr
{
	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		~OpenGLVertexArray();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual const bool IsBinded() override { return m_IsBinded; }
		virtual void SetBufferLayout(const BufferLayout& bufferLayout) override;

	private:
		uint32_t m_RendererID;
		bool m_IsBinded = false;
		BufferLayout m_BufferLayout;
	};
}