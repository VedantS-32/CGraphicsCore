#include "CGRpch.h"
#include "Renderer.h"

#include "Camera.h"
#include "CGR/Core/Application.h"

#include <glm/gtc/type_ptr.hpp>

static float offset = 0.0f;

namespace Cgr
{
	Renderer::Renderer()
		: m_AmbientLight(0.25f, 0.25f, 0.25f), m_LightPosition(1.0f, 1.0f, 40.0f)
	{
		m_VertexArray = VertexArray::Create();
		m_SSBO = ShaderStorageBuffer::Create();

		m_BufferLayout =
		{
			{ShaderDataType::Float3, "aPosition"},
			{ShaderDataType::Float3, "aNormal"},
			{ShaderDataType::Float2, "aTexCoord"},
			{ShaderDataType::Float3, "aTangent"}
		};

		m_VertexArray->Bind();
		m_WorldSettings = UniformBuffer::Create("WorldSettings");
		m_ModelCommons = UniformBuffer::Create("ModelCommons");
		m_ModelProps = UniformBuffer::Create("ModelProps");

		m_WorldSettings->SetData(0, sizeof(glm::vec3), glm::value_ptr(glm::vec3(1.0f)));
		m_WorldSettings->SetData(sizeof(glm::vec4), sizeof(glm::vec3), glm::value_ptr(m_AmbientLight));
		m_WorldSettings->SetData(sizeof(glm::vec4) * 2, sizeof(glm::vec3), glm::value_ptr(m_LightPosition));
	}

	void Renderer::OnUpdate(Camera& camera)
	{
		m_WorldSettings->SetData(0, sizeof(glm::vec3), glm::value_ptr(camera.GetPosition()));

		m_ModelCommons->SetData(0, sizeof(glm::mat4), glm::value_ptr(camera.GetViewMatrix()));
		m_ModelCommons->SetData(sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera.GetViewProjectionMatrix()));

		//offset += 0.025f;
		//if (offset >= 360.0f)
		//	offset = 0.0f;
		//m_LightPosition.x = 25 * glm::sin(offset);
		//m_LightPosition.y = 25 * glm::cos(offset);
		//m_WorldSettings->SetData(sizeof(glm::vec4) * 2, sizeof(glm::vec3), glm::value_ptr(m_LightPosition));

		//for (auto& model : m_Models)
		//{
		//	m_ModelProps->SetData(0, sizeof(glm::mat4), glm::value_ptr(model->GetModelMatrix()));
		//	model->DrawModel(m_VertexArray, m_BufferLayout, m_SSBO);
		//}
	}

	void Renderer::AddModel(Ref<Model> model)
	{
		//int currentMatIdx = -1;
		//for (auto& mesh : model->GetMeshes())
		//{
		//	if (currentMatIdx != mesh.GetMaterialIndex())
		//	{
		//		currentMatIdx = mesh.GetMaterialIndex();
		//		m_WorldSettings->SetBlockBinding(model->GetMaterial(currentMatIdx)->GetShader()->GetRendererID());
		//		m_ModelCommons->SetBlockBinding(model->GetMaterial(currentMatIdx)->GetShader()->GetRendererID());
		//		m_ModelProps->SetBlockBinding(model->GetMaterial(currentMatIdx)->GetShader()->GetRendererID());
		//	}
		//}
		//m_Models.emplace_back(model);
	}

	void Renderer::SetShaderBuffer(Ref<Shader> shader)
	{
		m_WorldSettings->SetBlockBinding(shader->GetRendererID());
		m_ModelCommons->SetBlockBinding(shader->GetRendererID());
		m_ModelProps->SetBlockBinding(shader->GetRendererID());
	}

	void Renderer::DrawModel(Ref<Model> model) const
	{
		model->DrawModel(m_VertexArray, m_BufferLayout, m_SSBO);
	}

	Ref<Renderer> Renderer::Create()
	{
		return CreateRef<Renderer>();
	}
}
