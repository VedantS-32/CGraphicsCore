#pragma once

#include "CGR/Core/Core.h"

namespace Cgr
{
	struct FramebufferSpecification
	{
		uint32_t Width, Height;
	};

	class CGR_API Framebuffer
	{
	public:
		virtual	~Framebuffer() {}

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Invalidate() = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetColorAttachmentRendererID() = 0;
		virtual uint32_t GetDepthAttachmentRendererID() = 0;

	public:
		static Ref<Framebuffer> Create();
	};
}
