#pragma once

#include "CGR/Renderer/FrameBuffer.h"
#include <glad/glad.h>

namespace Cgr
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Invalidate() override;
		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual uint32_t GetColorAttachmentRendererID() override { glBindTexture(GL_TEXTURE_2D, m_ColorAttachment); return m_ColorAttachment; }
		virtual uint32_t GetDepthAttachmentRendererID() override { glBindRenderbuffer(GL_RENDERBUFFER, m_DepthAttachment); return m_DepthAttachment; }

	private:
		FramebufferSpecification m_Specifications;

		uint32_t m_ColorAttachment, m_DepthAttachment;
		uint32_t m_RendererID;
	};
}