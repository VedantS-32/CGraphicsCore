#include "CGRpch.h"
#include "Renderer.h"

#include "Camera.h"

#include <glm/gtc/type_ptr.hpp>

namespace Cgr
{
	ModelRenderer::ModelRenderer(const Ref<VertexArray> vertexArray, Ref<ShaderStorageBuffer> SSBO)
		: m_VertexArray(vertexArray), m_SSBO(SSBO), m_AmbientLight(0.25f, 0.25f, 0.25f), m_LightPosition(1.0f, 1.0f, 5.0f)
	{
		//float verts[] =
		//{
		//	// Positions		// Normals			// TexCoord
		//	1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		//	1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		//	1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f
		//};
		//uint32_t indices[] =
		//{
		//	0, 1, 2, 2, 3, 0,  // front face
		//	4, 5, 6, 6, 7, 4,  // back face
		//	0, 4, 5, 5, 1, 0,  // left face
		//	1, 5, 6, 6, 2, 1,  // right face
		//	4, 7, 3, 3, 0, 4,  // bottom face
		//	2, 3, 7, 7, 6, 2  // top face
		//};
		//// Temporary vertex buffer for configuring vertex array layout
		//auto tempVbo = VertexBuffer::Create(sizeof(verts), verts);
		//auto tempIbo = IndexBuffer::Create(sizeof(indices) / sizeof(float), indices);

		//auto tempModel = Model::Create("Content/Model/Pot.fbx");

		m_BufferLayout =
		{
			{ShaderDataType::Float3, "aPosition"},
			{ShaderDataType::Float3, "aNormal"},
			{ShaderDataType::Float2, "aTexCoord"}
		};

		//m_VertexArray = VertexArray::Create();
		//m_VertexArray->SetBufferLayout(bufferLayout);
		//tempVbo->Unbind();

		m_VertexArray->Bind();
		m_WorldSettings = UniformBuffer::Create("WorldSettings");
		m_ModelCommons = UniformBuffer::Create("ModelCommons");
		m_ModelProps = UniformBuffer::Create("ModelProps");

		m_WorldSettings->SetData(0, sizeof(glm::vec3), glm::value_ptr(glm::vec3(1.0f)));
		m_WorldSettings->SetData(sizeof(glm::vec4), sizeof(glm::vec3), glm::value_ptr(m_AmbientLight));
		m_WorldSettings->SetData(sizeof(glm::vec4) * 2, sizeof(glm::vec3), glm::value_ptr(m_LightPosition));

		m_ShaderLibrary = ShaderLibrary::Create();
		m_ShaderLibrary->Add("Content/Shader/Phong.glsl");
	}

	void ModelRenderer::OnUpdate(Camera& camera)
	{
		//m_ShaderLibrary->Get("Cube")->Bind();
		m_WorldSettings->SetData(0, sizeof(glm::vec3), glm::value_ptr(camera.GetPosition()));

		m_ModelCommons->SetData(0, sizeof(glm::mat4), glm::value_ptr(camera.GetViewMatrix()));
		m_ModelCommons->SetData(sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera.GetViewProjectionMatrix()));

		for (auto& model : m_Models)
		{
			m_ModelProps->SetData(0, sizeof(glm::mat4), glm::value_ptr(model->GetModelMatrix()));
			model->DrawModel(m_VertexArray, m_BufferLayout, m_SSBO);
		}
	}

	void ModelRenderer::AddModel(const std::string& modelPath)
	{
		auto model = Model::Create(modelPath);
		int currentMatIdx = -1;
		for (auto& mesh : model->GetMeshes())
		{
			if (currentMatIdx != mesh.GetMaterialIndex())
			{
				model->AddMaterial(m_ShaderLibrary->Get("Phong"));
				//CGR_CORE_TRACE("Added material to model, index: {0}", mesh.GetMaterialIndex());
				currentMatIdx = mesh.GetMaterialIndex();
				m_WorldSettings->SetBlockBinding(model->GetMaterial(currentMatIdx)->GetShader()->GetRendererID());
				m_ModelCommons->SetBlockBinding(model->GetMaterial(currentMatIdx)->GetShader()->GetRendererID());
				m_ModelProps->SetBlockBinding(model->GetMaterial(currentMatIdx)->GetShader()->GetRendererID());
			}
		}
		m_Models.emplace_back(model);
	}

	void ModelRenderer::AddShader(const std::string& shaderPath)
	{
		m_ShaderLibrary->Add(shaderPath);
	}

	Ref<ModelRenderer> ModelRenderer::Create(const Ref<VertexArray> vertexArray, Ref<ShaderStorageBuffer> SSBO)
	{
		return CreateRef<ModelRenderer>(vertexArray, SSBO);
	}
}
