#pragma once

#include <CGR.h>

#include <glm/glm.hpp>

namespace Cgr
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer(const std::string& layerName);

		virtual void OnAttach() { CGR_TRACE("Attached {0} layer", m_LayerName); }
		virtual void OnDetach() { CGR_TRACE("Detached {0} layer", m_LayerName); }
		virtual void OnUpdate();
		virtual void OnUIRender();
		virtual void OnEvent(Event& e);

	private:
		Scope<BufferLayout> m_bufferLayout;
		Ref<VertexArray> m_vao;
		Ref<VertexBuffer> m_vbo;
		Ref<IndexBuffer> m_ibo;
		Ref<Shader> m_shader;

		glm::vec4 m_ClearColor;
	};
}