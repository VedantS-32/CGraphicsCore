#pragma once

#include "CGR/Renderer/Texture.h"
#include <glad/glad.h>

namespace Cgr
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path);
		OpenGLTexture2D(const TextureSpecification& spec, const void* data);
		OpenGLTexture2D(uint32_t width, uint32_t height);
		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual const std::string& GetFilepath() const override { return m_Path; }
		virtual const std::string& GetName() const override { return m_Name; }
		virtual void SetName(const std::string& name) { m_Name = name; }
		virtual uint32_t GetRendererID() const override { return m_RendererID; }

		virtual void SetData(void* data, uint32_t size) override;

		virtual void BindActiveTexture(uint32_t slot = 0) const override;
		virtual void Bind(uint32_t slot = 0) const override;
		virtual void Unbind() const override;

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
		}

	private:
		std::string m_Path;
		std::string m_Name;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
	};
}