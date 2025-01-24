#include "CGRpch.h"
#include "Renderer.h"

#include "Camera.h"
#include "CGR/Core/Application.h"

#include <glm/gtc/type_ptr.hpp>

static float offset = 0.0f;

namespace Cgr
{
	ModelRenderer::ModelRenderer(const Ref<VertexArray> vertexArray, Ref<ShaderStorageBuffer> SSBO)
		: m_VertexArray(vertexArray), m_SSBO(SSBO), m_AmbientLight(0.25f, 0.25f, 0.25f), m_LightPosition(1.0f, 1.0f, 40.0f)
	{
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

		m_ShaderLibrary = ShaderLibrary::Create();
		m_ShaderLibrary->Add("Content/Shader/Phong.glsl");
	}

	void ModelRenderer::OnUpdate(Camera& camera)
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

		for (auto& model : m_Models)
		{
			m_ModelProps->SetData(0, sizeof(glm::mat4), glm::value_ptr(model->GetModelMatrix()));
			model->DrawModel(m_VertexArray, m_BufferLayout, m_SSBO);
		}
	}

	void ModelRenderer::AddModel(const std::string& modelPath)
	{
		auto model = Model::Create(modelPath);

		// TODO: Temporary solution remove after completing model importer
		auto assetManager = Application::Get().GetAssetManager();
		auto materialHandle = assetManager->GetDefaultAssetHandle(AssetType::Material);
		auto material = assetManager->GetAsset<Material>(materialHandle);

		int currentMatIdx = -1;
		for (auto& mesh : model->GetMeshes())
		{
			if (currentMatIdx != mesh.GetMaterialIndex())
			{
				model->AddMaterial(material);
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
