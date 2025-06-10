#include "CGRpch.h"
#include "OpenGLEnvironmentMap.h"

#include "OpenGLBuffer.h"
#include "CGR/Renderer/Camera.h"
#include "CGR/Core/Application.h"

#include <glad/glad.h>
#include <stb_image.h>
#include <glm/ext/matrix_transform.hpp>

namespace Cgr
{
	namespace Utils
	{
		static GLenum SkyboxSideEnumToGL(SkyboxSide side)
		{
			switch (side)
			{
			case Cgr::SkyboxSide::NegativeX:
				return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
				break;
			case Cgr::SkyboxSide::NegativeY:
				return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
				break;
			case Cgr::SkyboxSide::NegativeZ:
				return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
				break;
			case Cgr::SkyboxSide::PositiveX:
				return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
				break;
			case Cgr::SkyboxSide::PositiveY:
				return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
				break;
			case Cgr::SkyboxSide::PositiveZ:
				return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
				break;
			default:
				return GL_NONE;
				break;
			}
		}

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

	OpenGLSkybox::OpenGLSkybox()
	{
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID.ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	OpenGLSkybox::~OpenGLSkybox()
	{
		glDeleteTextures(1, &m_RendererID.ID);
	}

	void OpenGLSkybox::Bind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		glBindTextureUnit(0, m_RendererID);
	}

	void OpenGLSkybox::Bind(uint32_t slot)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		glActiveTexture(m_RendererID);
		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLSkybox::UnBind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void OpenGLSkybox::SetTexture(SkyboxSide side, TextureSpecification spec, const void* data)
	{
		GLenum internalFormat = Utils::ToOpenGLTexInternalFormat(spec.Format);
		GLenum dataFormat = Utils::ToOpenGLTexDataFormat(spec.Format);
		GLenum target = Utils::SkyboxSideEnumToGL(side);
		glTexImage2D(target, 0, internalFormat, spec.Width, spec.Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLSkybox::UploadTextures()
	{
		auto assetManager = Application::Get().GetAssetManager();
		for (auto& [side, texHandle] : m_TextureHandles)
		{
			int width, height, channels;
			stbi_uc* data = nullptr;

			bool flipHorizontally = false;
			if (side == SkyboxSide::NegativeY || side == SkyboxSide::NegativeZ)
			{
				stbi_set_flip_vertically_on_load(0);
				flipHorizontally = true;
			}
			else
				stbi_set_flip_vertically_on_load(1);

			data = stbi_load(assetManager->GetFilePath(texHandle).string().c_str(), &width, &height, &channels, 0);
			if (!data)
			{
				data = stbi_load("Content/Texture/UVChecker.png", &width, &height, &channels, 0);
				CGR_CORE_ASSERT(data, "Failed to load image!");
			}

			if (flipHorizontally)
			{
				stbUtils::FlipImageHorizontally(data, width, height, channels);
			}
			if (side == SkyboxSide::NegativeX)
			{
				stbUtils::RotateImage90(data, width, height, channels, false);
			}
			if (side == SkyboxSide::PositiveX)
			{
				stbUtils::RotateImage90(data, width, height, channels, true);
			}

			TextureSpecification spec;
			spec.Width = width;
			spec.Height = height;
			if (channels == 4)
			{
				spec.Format = ImageFormat::RGBA8;
			}
			else if (channels == 3)
			{
				spec.Format = ImageFormat::RGB8;
			}

			SetTexture(side, spec, data);

			stbi_image_free(data);
		}
	}

	void OpenGLSkybox::SetShader(Ref<Shader> shader)
	{
		m_Shader = shader;
		Renderer::Get()->SetShaderBuffer(m_Shader);
	}

	void OpenGLSkybox::Render(Camera& camera)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		glBindTextureUnit(0, m_RendererID);
		m_Shader->Bind();
		m_Shader->SetMat4f("uProjection", camera.GetProjectionMatrix());
		m_Shader->SetMat4f("uView", camera.GetViewMatrix() * Renderer::Get()->m_SkyboxProps.RotationMatrix);
		m_Shader->Set1i("uSkybox", 0);

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
	}
}
