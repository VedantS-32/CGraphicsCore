#pragma once

#include "CGR/Renderer/Buffer.h"

namespace Cgr
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(int64_t size, const void* data, BufferDrawUsage usage);
		~OpenGLVertexBuffer();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void SetData(int64_t offset, int64_t size, const void* data) override;

	private:
		uint32_t m_RendererID;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t count, const uint32_t* indices, BufferDrawUsage usage);
		~OpenGLIndexBuffer();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual uint32_t GetCount() override { return m_Count; };
		virtual void SetData(int64_t offset, uint32_t count, const uint32_t* indices) override;

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};

	class OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(const std::string& blockName);
		~OpenGLUniformBuffer();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void SetBlockBinding(uint32_t shaderID) override;
		virtual void SetData(int64_t offset, int64_t size, const void* data) override;

	private:
		std::string m_BlockName;

	private:
		uint32_t m_BindingPoint;
		uint32_t m_RendererID;
	};


	class OpenGLShaderStorageBuffer : public ShaderStorageBuffer
	{
	public:
		OpenGLShaderStorageBuffer();
		~OpenGLShaderStorageBuffer();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void* GetDataPtr() override { return m_DataPtr; }
		virtual void SetData(int64_t offset, int64_t size, const void* data) override;

	private:
		uint32_t m_RendererID;
		void* m_DataPtr = nullptr;
	};
}