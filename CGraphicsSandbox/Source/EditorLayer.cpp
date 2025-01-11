#include "EditorLayer.h"

#include "CGR.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace Cgr
{
    static glm::mat4 transform{ 1.0f };
    static glm::vec3 sPos = { 0.0f, 0.0f, 0.0f };

	EditorLayer::EditorLayer(const std::string& layerName)
		: Layer(layerName), m_ClearColor(0.5f, 0.5f, 0.5f, 1.0f), m_Color(1.0f, 1.0f, 0.5f, 1.0f), m_Color2(0.65f, 0.5f, 0.85f, 1.0f), m_ViewportSize(1280, 720)
    {
		m_BufferLayout =
		{
			{ShaderDataType::Float3, "aPosition"},
			{ShaderDataType::Float3, "aNormal"},
            {ShaderDataType::Float2, "aTexCoord"}
		};

        auto tempModel = Model::Create("Content/Model/Cube.obj");
		m_VertexArray = VertexArray::Create();
        m_VertexArray->SetBufferLayout(m_BufferLayout);
        m_VertexArray->Unbind();
        tempModel->~Model();

        m_SSBO = ShaderStorageBuffer::Create();

        m_ModelRenderer = ModelRenderer::Create(m_VertexArray, m_SSBO);

        m_VertexArray->Bind();
        m_ModelRenderer->AddModel("Content/Model/Pot.fbx");
        m_ModelRenderer->AddModel("Content/Model/Cube.obj");

        m_Framebuffer = Framebuffer::Create();

		RenderCommand::SetClearColor(m_ClearColor);

        transform = glm::translate(glm::mat4(1.0f), sPos);
	}

    void EditorLayer::OnAttach()
    {
        CGR_TRACE("Attached {0} layer", m_LayerName);
        m_Camera.SetPerspective(60.0f);
        //m_Camera.SetOrthographic(10.0f);
    }

	void EditorLayer::OnUpdate(Timestep ts)
	{
        m_Framebuffer->Bind();

		RenderCommand::Clear();

        m_Camera.OnUpdate(ts);
        m_ModelRenderer->OnUpdate(m_Camera);

        m_Framebuffer->Unbind();
	}

	void EditorLayer::OnUIRender()
	{
        static bool dockSpaceOpen = true;
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        if (opt_fullscreen)
        {
            //m_ViewportSize = { viewport->WorkSize.x, viewport->WorkSize.y };
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockSpaceOpen, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        minWinSizeX = 370.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        style.WindowMinSize.x = minWinSizeX;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin("Viewport");

        auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
        auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
        auto viewportOffset = ImGui::GetWindowPos();

        m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
        m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();
        //Application::Get().GetUILayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        if ((m_ViewportSize != *((glm::vec2*)&viewportSize)) && (viewportSize.x > 0 && viewportSize.y > 0))
        {
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            //CGR_INFO("Viewport resized: {0}, {1}", m_ViewportSize.x, m_ViewportSize.y);
            m_ViewportSize = { viewportSize.x, viewportSize.y };
            m_Camera.SetViewportAspectRatio(viewportSize.x, viewportSize.y);
        }

        uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

        ImGui::End();
        ImGui::PopStyleVar();

        auto& models = m_ModelRenderer->GetModels();
        std::vector<const char*> modelList;
        for (auto& model : models)
        {
            modelList.push_back(model->GetName().c_str());
        }

        ImGui::Begin("Scene Graph");
        ImGui::ListBox("Entities", &m_CurrentEntity, modelList.data(), static_cast<int>(modelList.size()));
        ImGui::End();

        ImGui::Begin("World Settings");
        if (ImGui::ColorEdit4("Clear color", glm::value_ptr(m_ClearColor)))
            RenderCommand::SetClearColor(m_ClearColor);
        ImGui::End();

        ImGui::Begin("Properties");
        if(m_CurrentEntity != -1)
        {
            //auto modelUniformBuffer = models[m_CurrentEntity]->GetMeshes()[0].GetMaterial()->GetUniformBuffer();
            //modelUniformBuffer->SetBlockBinding(models[m_CurrentEntity]->GetMeshes()[0].GetMaterial()->GetShader()->GetRendererID());
            //if (ImGui::DragFloat3("translation", glm::value_ptr(sPos), 0.1f))
            //{
            //    modelUniformBuffer->SetData(0, sizeof(glm::mat4), glm::value_ptr(glm::translate(glm::mat4(1.0f), sPos)));
            //}

            auto& modelMatrix = models[m_CurrentEntity]->GetModelMatrix();
            sPos = modelMatrix[3];
            //modelUniformBuffer->SetBlockBinding(models[m_CurrentEntity]->GetMeshes()[0].GetMaterial()->GetShader()->GetRendererID());
            if (ImGui::DragFloat3("translation", glm::value_ptr(sPos), 0.1f))
            {
                modelMatrix = glm::translate(glm::mat4(1.0f), sPos);
            }

            auto& materialParams = models[m_CurrentEntity]->GetMeshes()[0].GetMaterial()->GetAllVariables();

            for (auto& param : materialParams)
            {
                switch (param->GetType())
                {
                case Cgr::ShaderDataType::None:
                    break;
                case Cgr::ShaderDataType::Float:
                    ImGui::DragFloat(param->GetName().c_str(), static_cast<float*>(param->GetValue()), 0.1f);
                    break;
                case Cgr::ShaderDataType::Float2:
                    ImGui::DragFloat2(param->GetName().c_str(), static_cast<float*>(param->GetValue()));
                    break;
                case Cgr::ShaderDataType::Float3:
                    ImGui::ColorEdit3(param->GetName().c_str(), static_cast<float*>(param->GetValue()));
                    break;
                case Cgr::ShaderDataType::Float4:
                    ImGui::ColorEdit4(param->GetName().c_str(), static_cast<float*>(param->GetValue()));
                    break;
                case Cgr::ShaderDataType::Mat3:
                    //ImGui::DragFloat2(param->GetName().c_str(), static_cast<float*>(param->GetValue()));
                    break;
                case Cgr::ShaderDataType::Mat4:
                    //ImGui::DragFloat2(param->GetName().c_str(), static_cast<float*>(param->GetValue()));
                    break;
                case Cgr::ShaderDataType::Int:
                    ImGui::DragInt(param->GetName().c_str(), static_cast<int*>(param->GetValue()));
                    break;
                case Cgr::ShaderDataType::Int2:
                    ImGui::DragInt2(param->GetName().c_str(), static_cast<int*>(param->GetValue()));
                    break;
                case Cgr::ShaderDataType::Int3:
                    ImGui::DragInt3(param->GetName().c_str(), static_cast<int*>(param->GetValue()));
                    break;
                case Cgr::ShaderDataType::Int4:
                    ImGui::DragInt4(param->GetName().c_str(), static_cast<int*>(param->GetValue()));
                    break;
                case Cgr::ShaderDataType::Bool:
                    ImGui::Checkbox(param->GetName().c_str(), static_cast<bool*>(param->GetValue()));
                    break;
                default:
                    break;
                }
            }
        }
        auto& pos = m_Camera.GetPosition();
        auto txt = std::format("Camera position x:{}, y:{}, z:{}", pos.x, pos.y, pos.z);
        ImGui::Text(txt.c_str());
        ImGui::End();

        ImGui::End();
    }

	void EditorLayer::OnEvent(Event& e)
	{
        m_Camera.OnEvent(e);
	}
}
