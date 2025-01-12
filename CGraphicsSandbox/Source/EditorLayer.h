#pragma once

#include <CGR.h>

#include <glm/glm.hpp>

namespace Cgr
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer(const std::string& layerName);

		virtual void OnAttach() override;
		virtual void OnDetach() override { CGR_TRACE("Detached {0} layer", m_LayerName); }
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnUIRender() override;
		virtual void OnEvent(Event& e) override;

	private:
		BufferLayout m_BufferLayout;
		Ref<VertexArray> m_VertexArray;
		Ref<VertexBuffer> m_vbo;
		Ref<IndexBuffer> m_ibo;
		Ref<Shader> m_shader;

		Ref<ShaderLibrary> m_ShaderLib;
		Ref<Material> m_Material;
		Ref<Model> m_Model;

		Ref<Framebuffer> m_Framebuffer;
		Camera m_Camera;
		Ref<ShaderStorageBuffer> m_SSBO;
		Ref<ModelRenderer> m_ModelRenderer;

		glm::vec4 m_ClearColor;
		glm::vec2 m_ViewportSize;
		glm::vec2 m_ViewportBounds[2];
		bool m_ViewportFocused;
		bool m_ViewportHovered;

		int m_PreviousEntity = -1;
		int m_CurrentEntity = -1;
	};
}