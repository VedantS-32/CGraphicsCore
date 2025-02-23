#include "CGRpch.h"
#include "Renderer.h"

#include "RenderCommand.h"
#include "Camera.h"
#include "CGR/Core/Application.h"
#include "EnvironmentMap.h"

#include <glm/gtc/type_ptr.hpp>

static float offset = 0.0f;

static float s_TriangleVertices[] = {
	-1.0f, -1.0f,   // Bottom-left
	 3.0f, -1.0f,   // Bottom-right (over-extended)
	-1.0f,  3.0f    // Top-left (over-extended)
};

namespace Cgr
{
	Renderer::Renderer()
		: m_AmbientLight(0.25f, 0.25f, 0.25f), m_LightPosition(1.0f, 1.0f, 40.0f)
	{
		m_WorldSettings = UniformBuffer::Create("WorldSettings");
		m_ModelCommons = UniformBuffer::Create("ModelCommons");
		m_ModelProps = UniformBuffer::Create("ModelProps");

		m_WorldSettings->SetData(0, sizeof(glm::vec3), glm::value_ptr(glm::vec3(1.0f)));
		m_WorldSettings->SetData(sizeof(glm::vec4), sizeof(glm::vec3), glm::value_ptr(m_AmbientLight));
		m_WorldSettings->SetData(sizeof(glm::vec4) * 2, sizeof(glm::vec3), glm::value_ptr(m_LightPosition));
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
		m_ENVMapVertexBuffer = VertexBuffer::Create(sizeof(s_TriangleVertices), s_TriangleVertices, BufferDrawUsage::StaticDraw);

		m_ENVLayout =
		{
			{ ShaderDataType::Float2, "aPosition" }
		};

		m_ENVMapVertexArray->SetBufferLayout(m_ENVLayout);

		auto assetManager = Application::Get().GetAssetManager();
		auto handle = assetManager->GetDefaultAssetHandle(AssetType::Skybox);
		m_Skybox = assetManager->GetAsset<Skybox>(handle);
	}

	void Renderer::OnUpdate(Camera& camera)
	{
		m_WorldSettings->SetData(0, sizeof(glm::vec3), glm::value_ptr(camera.GetPosition()));
		m_ModelCommons->SetData(0, sizeof(glm::mat4), glm::value_ptr(camera.GetViewMatrix()));
		m_ModelCommons->SetData(sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera.GetViewProjectionMatrix()));
	}

	void Renderer::RenderSkybox(Camera& camera)
	{
		RenderCommand::EnableDepthMask(false);
		m_ENVMapVertexArray->Bind();
		m_ENVMapVertexBuffer->Bind();
		m_ENVMapVertexArray->SetBufferLayout(m_ENVLayout);
		m_Skybox->Render(camera);
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
