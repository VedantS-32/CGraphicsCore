#pragma once

#include "CGR/Core/Core.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Model.h"

namespace Cgr
{
	class Camera;

	class CGR_API Renderer
	{
	public:
		Renderer();
		
		//Always call Renderer::OnUpdate before calling ActiveScene::OnUpdate, as it updates shader buffer
		void OnUpdate(Camera& camera);
		void AddModel(Ref<Model> model);
		void SetShaderBuffer(Ref<Shader> shader);
		//void AddShader(const std::string& shaderPath);
		Ref<UniformBuffer> GetModelCommonsUniformBuffer() { return m_ModelCommons; }
		Ref<UniformBuffer> GetModelPropsUniformBuffer() { return m_ModelProps; }
		Ref<VertexArray> GetVertexArray() { return m_VertexArray; }
		Ref<ShaderStorageBuffer> GetSSBO() { return m_SSBO; }
		std::vector<Ref<Model>>& GetModels() { return m_Models; }

		void DrawModel(Ref<Model> model) const;

	public:
		static Ref<Renderer> Create();

		// Temp
		Ref<UniformBuffer> m_WorldSettings;
		glm::vec3 m_AmbientLight;
		glm::vec3 m_LightPosition;

	private:
		BufferLayout m_BufferLayout;
		Ref<VertexArray> m_VertexArray;

		Ref<UniformBuffer> m_ModelCommons;
		Ref<UniformBuffer> m_ModelProps;
		Ref<ShaderStorageBuffer> m_SSBO;

		std::vector<Ref<Model>> m_Models;
	};
}