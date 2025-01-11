#include "CGRpch.h"
#include "Renderer.h"

#include "Camera.h"

#include <glm/gtc/type_ptr.hpp>

namespace Cgr
{
	ModelRenderer::ModelRenderer(const Ref<VertexArray> vertexArray, Ref<ShaderStorageBuffer> SSBO)
		: m_VertexArray(vertexArray), m_SSBO(SSBO)
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
		m_ModelCommons = UniformBuffer::Create("ModelCommons");
		m_ModelProps = UniformBuffer::Create("ModelProps");
		m_ShaderLibrary = ShaderLibrary::Create();
	}

	void ModelRenderer::OnUpdate(Camera& camera)
	{
		m_ShaderLibrary->Get("Cube")->Bind();
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
		for (auto& mesh : model->GetMeshes())
		{
			auto material = mesh.GetMaterial();
			material->SetShader(m_ShaderLibrary->Get("Cube"));
			glm::mat4 transform{ 1.0f };
			//material->AddVariable<ShaderMat4>("Transform", transform);
			//material->AddVariable<ShaderFloat4>("Color", color);
			m_ModelCommons->SetBlockBinding(material->GetShader()->GetRendererID());
			m_ModelProps->SetBlockBinding(material->GetShader()->GetRendererID());
			//material->GetUniformBuffer()->SetBlockBinding(material->GetShader()->GetRendererID());
			//material->GetUniformBuffer()->SetData(0, sizeof(glm::mat4), glm::value_ptr(transform));
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
