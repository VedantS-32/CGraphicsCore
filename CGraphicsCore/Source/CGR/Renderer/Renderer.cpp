#include "CGRpch.h"
#include "Renderer.h"

#include "RenderCommand.h"
#include "Camera.h"
#include "CGR/Core/Application.h"
#include "CGR/Asset/AssetManager.h"
#include "EnvironmentMap.h"

#include "glad/glad.h"

#include <glm/gtc/type_ptr.hpp>

static float offset = 0.0f;

static float s_CubeVertices[] = {
	// Front face
	-1.0f, -1.0f,  1.0f,  // Bottom-left
	 1.0f, -1.0f,  1.0f,  // Bottom-right
	 1.0f,  1.0f,  1.0f,  // Top-right
	-1.0f,  1.0f,  1.0f,  // Top-left

	// Back face
	-1.0f, -1.0f, -1.0f,  // Bottom-left
	 1.0f, -1.0f, -1.0f,  // Bottom-right
	 1.0f,  1.0f, -1.0f,  // Top-right
	-1.0f,  1.0f, -1.0f   // Top-left
};

static uint32_t s_CubeIndices[] = {
	// Front face
	0, 1, 2,
	2, 3, 0,

	// Right face
	1, 5, 6,
	6, 2, 1,

	// Back face
	5, 4, 7,
	7, 6, 5,

	// Left face
	4, 0, 3,
	3, 7, 4,

	// Top face
	3, 2, 6,
	6, 7, 3,

	// Bottom face
	4, 5, 1,
	1, 0, 4
};


namespace Cgr
{
	Renderer* Renderer::s_Renderer = nullptr;

	SkyboxProps::SkyboxProps()
		: Tint(1.0f, 1.0f, 1.0f), Intensity(1.0f), Rotation(200.0f)
	{
		ATTRIBUTE(Skybox Tint, Tint);
		ATTRIBUTE(Skybox Intensity, Intensity);
		ATTRIBUTE(Skybox Rotation, Rotation);
		//ATTRIBUTE(Reflection Rotation, ReflectionRotation);

		REFLECT();

		RotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		RotationMatrix = glm::rotate(RotationMatrix, glm::radians(Rotation), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	void SkyboxProps::OnAttributeChange()
	{
		RotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		RotationMatrix = glm::rotate(RotationMatrix, glm::radians(Rotation), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	Renderer::Renderer()
		: m_AmbientLight(0.25f, 0.25f, 0.25f), m_LightPosition(1.0f, 1.0f, 40.0f)
	{
		s_Renderer = this;

		m_WorldSettings = UniformBuffer::Create("WorldSettings");
		m_ModelCommons = UniformBuffer::Create("ModelCommons");
		m_ModelProps = UniformBuffer::Create("ModelProps");

		auto rotationMatrix = m_SkyboxProps.RotationMatrix;
		auto& reflectionRotation = m_SkyboxProps.ReflectionRotation;
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(reflectionRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(reflectionRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(reflectionRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		m_ModelCommons->SetData(sizeof(glm::mat4) * 2, sizeof(glm::mat3), glm::value_ptr(glm::mat3(rotationMatrix)));

		m_WorldSettings->SetData(0, sizeof(glm::vec3), glm::value_ptr(glm::vec3(1.0f)));
		m_WorldSettings->SetData(sizeof(glm::vec4), sizeof(glm::vec3), glm::value_ptr(m_AmbientLight));
		m_WorldSettings->SetData(sizeof(glm::vec4) * 2, sizeof(glm::vec3), glm::value_ptr(m_LightPosition));
		m_WorldSettings->SetData(sizeof(glm::vec4) * 3, sizeof(glm::vec3), &m_SkyboxProps.Tint);
		
		// I don't know why frame capture is showing this offset
		m_WorldSettings->SetData((sizeof(glm::vec4) * 3) + sizeof(glm::vec3), sizeof(float), &m_SkyboxProps.Intensity);
	}

	void Renderer::Init()
	{
		m_ModelVertexArray = VertexArray::Create();
		m_SSBO = ShaderStorageBuffer::Create();

		m_BufferLayout =
		{
			{ ShaderDataType::Float3, "aPosition" },
			{ ShaderDataType::Float3, "aNormal" },
			{ ShaderDataType::Float2, "aTexCoord" },
			{ ShaderDataType::Float3, "aTangent" }
		};

		m_ENVMapVertexArray = VertexArray::Create();
		m_ENVMapVertexBuffer = VertexBuffer::Create(sizeof(s_CubeVertices), s_CubeVertices, BufferDrawUsage::StaticDraw);
		m_ENVMapIndexBuffer = IndexBuffer::Create(sizeof(s_CubeIndices), s_CubeIndices, BufferDrawUsage::StaticDraw);

		m_ENVLayout =
		{
			{ ShaderDataType::Float3, "aPosition" }
		};

		m_ENVMapVertexArray->SetBufferLayout(m_ENVLayout);

		auto assetManager = Application::Get().GetAssetManager();
		auto handle = assetManager->GetDefaultAssetHandle(AssetType::Skybox);
		m_Skybox = assetManager->GetAsset<Skybox>(handle);
	}

	Renderer* Renderer::Get()
	{
		return s_Renderer;
	}

	void Renderer::OnUpdate(Camera& camera)
	{
		m_ModelCommons->SetData(0, sizeof(glm::mat4), glm::value_ptr(camera.GetViewMatrix()));
		m_ModelCommons->SetData(sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera.GetViewProjectionMatrix()));

		auto& rotationMatrix = m_SkyboxProps.RotationMatrix;
		//auto& reflectionRotation = m_SkyboxProps.ReflectionRotation;
		//rotationMatrix = glm::rotate(rotationMatrix, glm::radians(reflectionRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		//rotationMatrix = glm::rotate(rotationMatrix, glm::radians(reflectionRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		//rotationMatrix = glm::rotate(rotationMatrix, glm::radians(reflectionRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		m_ModelCommons->SetData(sizeof(glm::mat4) * 2, sizeof(glm::mat3), glm::value_ptr(glm::mat3(rotationMatrix)));

		m_WorldSettings->SetData(0, sizeof(glm::vec3), glm::value_ptr(camera.GetPosition()));
		m_WorldSettings->SetData(sizeof(glm::vec4) * 3, sizeof(glm::vec3), &m_SkyboxProps.Tint);

		// I don't know why frame capture is showing this offset
		m_WorldSettings->SetData((sizeof(glm::vec4) * 3) + sizeof(glm::vec3), sizeof(float), &m_SkyboxProps.Intensity);
	}

	void Renderer::BindSkybox()
	{
		m_Skybox->Bind(15);
	}

	void Renderer::RenderSkybox(Camera& camera)
	{
		RenderCommand::EnableDepthMask(false);
		glDepthFunc(GL_LEQUAL);
		m_ENVMapVertexArray->Bind();
		m_ENVMapVertexBuffer->Bind();
		m_ENVMapIndexBuffer->Bind();
		m_Skybox->Render(camera);
		glDepthFunc(GL_LESS);
		RenderCommand::EnableDepthMask(true);
	}

	void Renderer::SetShaderBuffer(Ref<Shader> shader)
	{
		m_WorldSettings->SetBlockBinding(shader->GetRendererID());
		m_ModelCommons->SetBlockBinding(shader->GetRendererID());
		m_ModelProps->SetBlockBinding(shader->GetRendererID());
	}

	void Renderer::BindModelVertexArray() const
	{
		m_ModelVertexArray->Bind();
	}

	void Renderer::DrawModel(Ref<Model> model) const
	{
		model->DrawModel(m_ModelVertexArray, m_BufferLayout, m_SSBO);
	}

	Ref<Renderer> Renderer::Create()
	{
		return CreateRef<Renderer>();
	}
}
