#pragma once

#include "CGR/Core/Core.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Model.h"

namespace Cgr
{
	class Camera;

	class CGR_API ModelRenderer
	{
	public:
		ModelRenderer(const Ref<VertexArray> vertexArray, Ref<ShaderStorageBuffer> SSBO);

		void OnUpdate(Camera& camera);
		void AddModel(const std::string& modelPath);
		void AddShader(const std::string& shaderPath);
		Ref<UniformBuffer> GetModelPropsUniformBuffer() { return m_ModelProps; }
		std::vector<Ref<Model>>& GetModels() { return m_Models; }

	public:
		static Ref<ModelRenderer> Create(const Ref<VertexArray> vertexArray, Ref<ShaderStorageBuffer> SSBO);

	private:
		BufferLayout m_BufferLayout;
		Ref<VertexArray> m_VertexArray;
		Ref<ShaderLibrary> m_ShaderLibrary;
		Ref<UniformBuffer> m_ModelCommons;
		Ref<UniformBuffer> m_ModelProps;
		Ref<ShaderStorageBuffer> m_SSBO;

		std::vector<Ref<Model>> m_Models;
	};
}