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
		REFLECT();

		ATTRIBUTE("Skybox Rotation", m_Rotation);
		ATTRIBUTE("Skybox Intensity", m_Intensity);
		ATTRIBUTE("Redness", m_Red);

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	OpenGLSkybox::~OpenGLSkybox()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLSkybox::Bind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		glBindTextureUnit(0, m_RendererID);
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

	void OpenGLSkybox::SetShader(Ref<Shader> shader)
	{
		m_Shader = shader;
	}

	void OpenGLSkybox::Render(Camera& camera)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		glBindTextureUnit(0, m_RendererID);
		m_Shader->Bind();
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(m_Rotation), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 inverseVP = glm::inverse(camera.GetViewProjectionMatrix() * rotationMatrix);
		m_Shader->SetMat4f("uInverseVP", inverseVP);
		m_Shader->Set1i("uSkybox", 0);
		m_Shader->Set1f("uIntensity", m_Intensity);
		m_Shader->Set1f("uRed", m_Red);

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	void OpenGLSkybox::OnAttributeChange()
	{
		
	}
}
