#pragma once

#include "CGR/Renderer/Buffer.h"

namespace Cgr
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(int64_t size, const void* data, BufferDrawUsage usage);

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void SetData(int64_t offset, int64_t size, const void* data) override;

	private:
		uint32_t m_RendererID;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t count, uint32_t* indices, BufferDrawUsage usage);

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual uint32_t GetCount() override { return m_Count; };
		virtual void SetData(int64_t offset, uint32_t count, uint32_t* indices) override;

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};
}