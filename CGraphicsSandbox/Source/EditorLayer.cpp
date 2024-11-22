#include "EditorLayer.h"

#include "CGR.h"

namespace Cgr
{
	EditorLayer::EditorLayer(const std::string& layerName)
		: Layer(layerName), m_ClearColor(0.1f, 0.1f, 0.15f, 1.0f)
	{
		float vertices[] =
		{
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
			 0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
			-0.5f,  0.5f, 1.0f, 0.0f, 0.0f
		};

		uint32_t indices[] =
		{
			0, 1, 2,
			2, 3, 0
		};

		BufferLayout bufferLayout =
		{
			{ShaderDataType::Float2, "aPosition"},
			{ShaderDataType::Float3, "aColor"}
		};


		m_vbo = VertexBuffer::Create(sizeof(vertices), vertices);
		m_ibo = IndexBuffer::Create(6, indices);

		m_vao = VertexArray::Create(bufferLayout);

		m_shader = Shader::Create("Content/Shader/Triangle.glsl");

		RenderCommand::SetClearColor(m_ClearColor);
	}

	void EditorLayer::OnUpdate()
	{
		RenderCommand::Clear();
		RenderCommand::DrawIndexed(m_vao, m_ibo->GetCount());
	}

	void EditorLayer::OnUIRender()
	{
	}

	void EditorLayer::OnEvent(Event& e)
	{
	}
}
