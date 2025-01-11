#pragma once

#include "CGR/Core/Core.h"
#include "CGR/Renderer/Buffer.h"

namespace Cgr
{
	class CGR_API VertexArray
	{
	public:
		virtual ~VertexArray() {}

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void SetBufferLayout(const BufferLayout& bufferLayout) = 0;

	public:
		static Ref<VertexArray> Create();
	};
}
