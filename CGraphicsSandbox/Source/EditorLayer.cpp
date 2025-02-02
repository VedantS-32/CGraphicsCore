#include "EditorLayer.h"

#include "CGR.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>

namespace Cgr
{
    static glm::mat4 transform{ 1.0f };
    static glm::vec3 sPos = { 0.0f, 0.0f, 0.0f };
    static int meshIdx = 0;

	EditorLayer::EditorLayer(const std::string& layerName)
		: Layer(layerName), m_ClearColor(0.5f, 0.5f, 0.5f, 1.0f), m_ViewportSize(1280, 720)
    {
		m_VertexArray = VertexArray::Create();

        m_SSBO = ShaderStorageBuffer::Create();

        m_ModelRenderer = ModelRenderer::Create(m_VertexArray, m_SSBO);
        m_VertexArray->Bind();

        m_AssetManager = Application::Get().GetAssetManager();
        m_DefaultTexture = m_AssetManager->GetAsset<Texture2D>(m_AssetManager->GetDefaultAssetHandle(AssetType::Texture2D));
        auto handle = m_AssetManager->ImportAsset("Content/Model/Cube.csmesh");
        m_ModelRenderer->AddModel(m_AssetManager->GetAsset<Model>(handle));

        handle = m_AssetManager->ImportAsset("Content/Model/Pot.fbx");
        m_ModelRenderer->AddModel(m_AssetManager->GetAsset<Model>(handle));

        m_Framebuffer = Framebuffer::Create();

		RenderCommand::SetClearColor(m_ClearColor);

        transform = glm::translate(glm::mat4(1.0f), sPos);

	}

    EditorLayer::~EditorLayer()
    {
        delete m_ContentBrowserPanel;
    }

    void EditorLayer::OnAttach()
    {
        CGR_TRACE("Attached {0} layer", m_LayerName);
        m_ContentBrowserPanel = new ContentBrowserPanel;
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

        m_ContentBrowserPanel->OnImGuiRender();

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
            //CGR_INFO("Viewport resized: {0}, {1}", m_ViewportSize.x, m_ViewportSize.y);
            m_ViewportSize = { viewportSize.x, viewportSize.y };
            m_Camera.SetViewportAspectRatio(viewportSize.x, viewportSize.y);
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }

        uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

        ImGui::End();
        ImGui::PopStyleVar();

        auto& models = m_ModelRenderer->GetModels();
        std::vector<const char*> modelList;
        for (auto& model : models)
        {
            modelList.push_back(model->Name.c_str());
        }

        ImGui::Begin("Scene Graph");
        ImGui::ListBox("Entities", &m_CurrentEntity, modelList.data(), static_cast<int>(modelList.size()));
        ImGui::End();

        ImGui::Begin("World Settings");
        auto& pos = m_Camera.GetPosition();
        auto txt = std::format("Camera position x:{}, y:{}, z:{}", pos.x, pos.y, pos.z);
        ImGui::Text(txt.c_str());
        if (ImGui::ColorEdit4("Clear color", glm::value_ptr(m_ClearColor)))
            RenderCommand::SetClearColor(m_ClearColor);
        if(ImGui::ColorEdit3("AmbientLight", glm::value_ptr(m_ModelRenderer->m_AmbientLight)))
            m_ModelRenderer->m_WorldSettings->SetData(sizeof(glm::vec4), sizeof(glm::vec3), glm::value_ptr(m_ModelRenderer->m_AmbientLight));
        if(ImGui::DragFloat3("LightPosition", glm::value_ptr(m_ModelRenderer->m_LightPosition)))
            m_ModelRenderer->m_WorldSettings->SetData(sizeof(glm::vec4) * 2, sizeof(glm::vec3), glm::value_ptr(m_ModelRenderer->m_LightPosition));
        ImGui::End();

        ImGui::Begin("Properties");
        if(m_CurrentEntity != -1)
        {
            if (m_CurrentEntity != m_PreviousEntity)
            {
                meshIdx = 0;
                m_PreviousEntity = m_CurrentEntity;
            }

            auto& modelMatrix = models[m_CurrentEntity]->GetModelMatrix();
            sPos = modelMatrix[3];

            if (ImGui::DragFloat3("translation", glm::value_ptr(sPos), 0.1f))
            {
                modelMatrix = glm::translate(glm::mat4(1.0f), sPos);
            }

            auto& name = models[m_CurrentEntity]->Name;
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::Text("Model");
            const auto& iconMap = m_ContentBrowserPanel->GetIconMap();
            ImGui::ImageButton(name.c_str(), reinterpret_cast<void*>(static_cast<uintptr_t>(iconMap.at("Model")->GetRendererID())), { 98, 98 }, { 0, 1 }, { 1, 0 });
            ImGui::PopStyleColor();

            auto& model = models[m_CurrentEntity];

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    AssetHandle handle = *reinterpret_cast<const uint64_t*>(payload->Data);
                    model = m_AssetManager->GetAsset<Model>(handle);
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Text(name.c_str());
            if (ImGui::Button("Save", { 100.0f, 24.0f }))
            {
                ModelSerializer serializer(model);
                serializer.Serialize(m_AssetManager->GetFilePath(model->Handle));
            }

            if (ImGui::Button("Load", { 100.0f, 24.0f }))
            {
                ModelSerializer serializer(model);
                serializer.Deserialize(m_AssetManager->GetFilePath(model->Handle));
            }
            ImGui::EndGroup();
            
            std::vector<const char*> meshList;
            auto& meshes = models[m_CurrentEntity]->GetMeshes();

            ImGui::Text("Material");
            for (auto& mesh : meshes)
            {
                meshList.push_back(std::format("Index: {}", mesh.GetMaterialIndex()).c_str());
            }

            ImGui::ListBox("Material Index", &meshIdx, meshList.data(), static_cast<int>(meshList.size()));

            auto material = models[m_CurrentEntity]->GetMaterial(meshIdx);

            auto& matName = material->Name;
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::ImageButton(matName.c_str(), reinterpret_cast<void*>(static_cast<uintptr_t>(iconMap.at("Material")->GetRendererID())), {98, 98}, {0, 1}, {1, 0});
            ImGui::PopStyleColor();

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    AssetHandle handle = *reinterpret_cast<const uint64_t*>(payload->Data);
                    models[m_CurrentEntity]->SetMaterial(meshIdx, m_AssetManager->GetAsset<Material>(handle));
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Text(matName.c_str());
            if (ImGui::Button("Save", { 100.0f, 24.0f }))
            {
                MaterialSerializer serializer(material);
                serializer.Serialize(m_AssetManager->GetFilePath(material->Handle));
            }

            if (ImGui::Button("Load", { 100.0f, 24.0f }))
            {
                MaterialSerializer serializer(material);
                serializer.Deserialize(m_AssetManager->GetFilePath(material->Handle));
            }
            ImGui::EndGroup();

            auto& shaderName = material->GetShader()->Name;
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::Text("Shader");
            ImGui::ImageButton(shaderName.c_str(), reinterpret_cast<void*>(static_cast<uintptr_t>(iconMap.at("Shader")->GetRendererID())), { 98, 98 }, { 0, 1 }, { 1, 0 });
            ImGui::PopStyleColor();

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    AssetHandle handle = *reinterpret_cast<const uint64_t*>(payload->Data);
                    auto shader = m_AssetManager->GetAsset<Shader>(handle);
                    shader->Recompile();
                    m_ModelRenderer->SetShaderBuffer(shader);
                    material->SetShader(shader);
                    shader->ExtractSSBOParameters(material.get());
                    shader->UpdateSSBOParameters(material.get(), m_SSBO);
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Text(shaderName.c_str());
            if (ImGui::Button("Recompile", { 100.0f, 24.0f }))
            {
                auto shader = material->GetShader();
                shader->Recompile();
                m_ModelRenderer->SetShaderBuffer(shader);
                shader->ExtractSSBOParameters(material.get());
                shader->UpdateSSBOParameters(material.get(), m_SSBO);
            }
            ImGui::EndGroup();

            auto& textures = material->GetAllTextures();
            bool removeTex = false;
            UUID texuuid;
            ImGui::Text("Material Parameters");
            int id = 0;
            for (auto& [uuid, texture] : textures)
            {
                auto& texName = texture->GetName();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::ImageButton(texName.c_str(), reinterpret_cast<void*>(static_cast<uintptr_t>(texture->GetRendererID())), { 98, 98 }, { 0, 1 }, { 1, 0 });
                ImGui::PopStyleColor();

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        AssetHandle handle = *reinterpret_cast<const uint64_t*>(payload->Data);
                        texture = m_AssetManager->GetAsset<Texture2D>(handle);
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::SameLine();
                ImGui::BeginGroup();
                ImGui::Text(texName.c_str());
                
                ImGui::PushID(id);
                if (ImGui::Button("Remove", { 100.0f, 24.0f }))
                {
                    texuuid = uuid;
                    removeTex = true;
                }
                id++;
                ImGui::PopID();
                ImGui::EndGroup();
            }

            if (removeTex)
            {
                if (textures.size() <= 1)
                {
                    textures[texuuid] = m_DefaultTexture;
                }
                else
                    textures.erase(texuuid);
            }
            removeTex = false;

            ImGui::Spacing();
            ImGui::Spacing();
            float buttonX = ImGui::GetStyle().WindowPadding.x * 1.75f;
            ImGui::SetCursorPosX(buttonX);
            if (ImGui::Button("Add Texture", { 100.0f, 24.0f }))
            {
                material->AddTexture(texuuid, m_DefaultTexture);
            }
            ImGui::Spacing();
            ImGui::Spacing();

            auto& materialParams = material->GetAllVariables();
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

        ImGui::End();

        ImGui::End();
    }

	void EditorLayer::OnEvent(Event& e)
	{
        m_Camera.OnEvent(e);
	}
}
