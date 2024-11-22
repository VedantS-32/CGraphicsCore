#include "CGRpch.h"

#include "Buffer.h"
#include "RendererAPI.h"

#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Cgr
{
	Ref<VertexBuffer> VertexBuffer::Create(int64_t size, const void* data, BufferDrawUsage usage)
	{
		switch (RendererAPI::GetAPI())
		{
		case Cgr::API::None:
			CGR_CORE_ASSERT(false, "No Graphics API selected");
			break;
		case Cgr::API::OpenGL:
			return CreateRef<OpenGLVertexBuffer>(size, data, usage);
			break;
		default:
			break;
		}

		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t count, uint32_t* indices, BufferDrawUsage usage)
	{
		switch (RendererAPI::GetAPI())
		{
		case Cgr::API::None:
			CGR_CORE_ASSERT(false, "No Graphics API selected");
			break;
		case Cgr::API::OpenGL:
			return CreateRef<OpenGLIndexBuffer>(count, indices, usage);
			break;
		default:
			break;
		}

		return nullptr;
	}
}
