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
		static GLenum CubeMapSideEnumToGL(CubeMapSide side)
		{
			switch (side)
			{
			case Cgr::CubeMapSide::NegativeX:
				return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
				break;
			case Cgr::CubeMapSide::NegativeY:
				return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
				break;
			case Cgr::CubeMapSide::NegativeZ:
				return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
				break;
			case Cgr::CubeMapSide::PositiveX:
				return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
				break;
			case Cgr::CubeMapSide::PositiveY:
				return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
				break;
			case Cgr::CubeMapSide::PositiveZ:
				return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
				break;
			default:
				return GL_NONE;
				break;
			}
		}

		static void LoadCubeMapTextures()
		{
			const char* path[] = {
				"Content/Texture/CubeMap/MilkyWay/px.png",
				"Content/Texture/CubeMap/MilkyWay/nx.png",
				"Content/Texture/CubeMap/MilkyWay/py.png",
				"Content/Texture/CubeMap/MilkyWay/ny.png",
				"Content/Texture/CubeMap/MilkyWay/pz.png",
				"Content/Texture/CubeMap/MilkyWay/nz.png"
			};

			for (int i = 0; i < 6; i++)
			{
				int width, height, channels;
				stbi_uc* data = nullptr;

				stbi_set_flip_vertically_on_load(1);
				{
					data = stbi_load(path[i], &width, &height, &channels, 0);
				}
				if (!data)
				{
					data = stbi_load("asset/texture/UVChecker.png", &width, &height, &channels, 0);
					CGR_CORE_ASSERT(data, "Failed to load image!");
				}

				GLenum internalFormat = 0, dataFormat = 0;
				if (channels == 4)
				{
					internalFormat = GL_RGBA8;
					dataFormat = GL_RGBA;
				}
				else if (channels == 3)
				{
					internalFormat = GL_RGB8;
					dataFormat = GL_RGB;
				}

				CGR_CORE_ASSERT(internalFormat & dataFormat, "Image format not supported!");

				GLenum side = CubeMapSideEnumToGL(static_cast<CubeMapSide>(i));
				glTexImage2D(side, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);

				stbi_image_free(data);
			}
		}
	}

	OpenGLCubeMap::OpenGLCubeMap()
	{
		float triangleVertices[] = {
			-1.0f, -1.0f,   // Bottom-left
			 3.0f, -1.0f,   // Bottom-right (over-extended)
			-1.0f,  3.0f    // Top-left (over-extended)
		};

		m_ENVMapVertexArray = VertexArray::Create();
		BufferLayout layout =
		{
			{ ShaderDataType::Float2, "aPosition"}
		};

		m_ENVMapVertexBuffer = VertexBuffer::Create(sizeof(triangleVertices), triangleVertices, BufferDrawUsage::StaticDraw);
		m_ENVMapVertexArray->SetBufferLayout(layout);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		Utils::LoadCubeMapTextures();

		// Use AssetManager to import env map material
		auto assetManager = Application::Get().GetAssetManager();
		auto handle = assetManager->ImportAsset("Content/Shader/CubeMap.glsl");
		m_Shader = assetManager->GetAsset<Shader>(handle);
		//m_ENVMapMaterial = Material::Create(shader);
	}

	OpenGLCubeMap::~OpenGLCubeMap()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLCubeMap::Bind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
	}

	void OpenGLCubeMap::UnBind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void OpenGLCubeMap::Render(Camera& camera)
	{
		m_ENVMapVertexArray->Bind();
		m_ENVMapVertexBuffer->Bind();
		//auto shader = m_ENVMapMaterial->GetShader();
		m_Shader->Bind();
		float rotation = -90.0f;
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 inverseVP = glm::inverse(camera.GetViewProjectionMatrix() * rotationMatrix);
		m_Shader->SetMat4f("uInverseVP", inverseVP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		m_Shader->Set1i("uSkybox", m_RendererID);

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}
