#pragma once

#include "CGR/Renderer/FrameBuffer.h"
#include <glad/glad.h>

namespace Cgr
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		~OpenGLFramebuffer();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Invalidate() override;
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;
		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override { CGR_CORE_ASSERT(index < m_ColorAttachments.size()); return m_ColorAttachments[index]; }
		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specifications; }

	private:
		FramebufferSpecification m_Specifications;
		std::vector<FramebufferTextureFormat> m_ColorAttachmentSpecifications;
		FramebufferTextureFormat m_DepthAttachmentSpecification = FramebufferTextureFormat::None;
		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment = 0;

		uint32_t m_RendererID;
	};
}