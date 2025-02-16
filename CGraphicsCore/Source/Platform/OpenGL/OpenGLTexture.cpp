#include "CGRpch.h"

#include "OpenGLTexture.h"

#include <stb_image.h>

namespace Cgr
{
	namespace Utils
	{
		static GLenum ToOpenGLTexInternalFormat(ImageFormat format)
		{
			switch (format)
			{
			case Cgr::ImageFormat::None:
				CGR_CORE_ERROR("Invalid format");
				break;
			case Cgr::ImageFormat::R8:
				CGR_CORE_ASSERT(false, "Not implemented yet")
				break;
			case Cgr::ImageFormat::RGB8:
				return GL_RGB8;
				break;
			case Cgr::ImageFormat::RGBA8:
				return GL_RGBA8;
				break;
			case Cgr::ImageFormat::RGBA16F:
				return GL_RGBA16F;
				break;
			case Cgr::ImageFormat::RGBA32F:
				return GL_RGBA32F;
				break;
			default:
				CGR_CORE_ERROR("Invalid format");
				return GL_NONE;
				break;
			}

			return GL_NONE;
		}

		static GLenum ToOpenGLTexDataFormat(ImageFormat format)
		{
			switch (format)
			{
			case Cgr::ImageFormat::None:
				CGR_CORE_ERROR("Invalid format");
				break;
			case Cgr::ImageFormat::R8:
				CGR_CORE_ASSERT(false, "Not implemented yet")
					break;
			case Cgr::ImageFormat::RGB8:
				return GL_RGB;
				break;
			case Cgr::ImageFormat::RGBA8:
				return GL_RGBA;
				break;
			case Cgr::ImageFormat::RGBA16F:
				return GL_RGBA;
				break;
			case Cgr::ImageFormat::RGBA32F:
				return GL_RGBA;
				break;
			default:
				CGR_CORE_ERROR("Invalid format");
				return GL_NONE;
				break;
			}

			return GL_NONE;
		}
	}

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height), m_Name("Default")
	{
		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& spec, const void* data)
		: m_Width(spec.Width), m_Height(spec.Height)
	{
		m_InternalFormat = Utils::ToOpenGLTexInternalFormat(spec.Format);
		m_DataFormat = Utils::ToOpenGLTexDataFormat(spec.Format);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		CGR_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Bind() const
	{
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
	}

	void OpenGLTexture2D::BindActiveTexture(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
	}

	void OpenGLTexture2D::Unbind() const
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}