#include "EditorLayer.h"

#include "CGR.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

namespace Cgr
{
    extern const std::filesystem::path s_AssetPath;
	EditorLayer::EditorLayer(const std::string& layerName)
		: Layer(layerName), m_ClearColor(0.5f, 0.5f, 0.5f, 1.0f), m_ViewportSize(1280, 720)
    {
        m_AssetManager = Application::Get().GetAssetManager();
        m_Renderer = Application::Get().GetRenderer();
        m_ReflectionSystem = Application::Get().GetReflectionSystem();

        m_ActiveScene = CreateRef<Scene>();
        auto entity = m_ActiveScene->CreateEntity("FirstEntity");

        auto handle = m_AssetManager->ImportAsset("Content/Model/Cube.csmesh");
        auto& model = entity.AddComponent<ModelComponent>();
        model.SetModel(m_AssetManager->GetAsset<Model>(handle));

        handle = m_AssetManager->ImportAsset("Content/Model/Pot.fbx");
        entity = m_ActiveScene->CreateEntity("SecondEntity");
        auto& model1 = entity.AddComponent<ModelComponent>();
        model1.SetModel(m_AssetManager->GetAsset<Model>(handle));
        auto& transform = entity.GetComponent<TransformComponent>();

        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);

		RenderCommand::SetClearColor(m_ClearColor);
        m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
	}

    EditorLayer::~EditorLayer()
    {
        delete m_ContentBrowserPanel;
    }

    void EditorLayer::OnAttach()
    {
        CGR_TRACE("Attached {0} layer", m_LayerName);
        m_ContentBrowserPanel = new ContentBrowserPanel;
        m_SceneGraphPanel = new SceneGraphPanel(m_ActiveScene, m_ContentBrowserPanel);
        m_Camera.SetPerspective(60.0f);
        //m_Camera.SetOrthographic(10.0f);
    }

	void EditorLayer::OnUpdate(Timestep ts)
	{
        if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
            m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
            (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
        {
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_Camera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
            m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
        }

        m_Framebuffer->Bind();

		RenderCommand::Clear();

        // Clear our entity ID attachment to -1
        m_Framebuffer->ClearAttachment(1, -1);

        m_Camera.OnUpdate(ts);

        // Updating shader buffers
        m_Renderer->OnUpdate(m_Camera);

        // Drawing models
        m_Renderer->BindModelVertexArray();
        m_ActiveScene->OnUpdate(ts, m_Camera);

        // Rendering Skybox
        m_Renderer->RenderSkybox(m_Camera);

        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;
        glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
        my = viewportSize.y - my;
        int mouseX = (int)mx;
        int mouseY = (int)my;

        if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
        {
            int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
            m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
        }

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


        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                // Disabling fullscreen would allow the window to be moved to the front of other windows,
                // which we can't undo at the moment without finer window depth/z control.
                ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
                ImGui::MenuItem("Padding", NULL, &opt_padding);
                ImGui::Separator();

                if (ImGui::MenuItem("New", "Ctrl+N"))
                    NewScene();

                if (ImGui::MenuItem("Open", "Ctrl+O"))
                    OpenScene();

                if (ImGui::MenuItem("Save", "Ctrl+Shift+S"))
                    SaveSceneAs();

                if (ImGui::MenuItem("Exit")) { Application::Get().Close(); }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin("Viewport");

        auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
        auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
        auto viewportOffset = ImGui::GetWindowPos();

        m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
        m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();
        Application::Get().GetUILayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportSize.x, viewportSize.y };
        if ((m_ViewportSize != *((glm::vec2*)&viewportSize)) && (viewportSize.x > 0 && viewportSize.y > 0))
        {
            //CGR_INFO("Viewport resized: {0}, {1}", m_ViewportSize.x, m_ViewportSize.y);
            m_Camera.SetViewportSize(viewportSize.x, viewportSize.y);
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }

        uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                auto path = static_cast<const char*>(payload->Data);
                OpenScene(std::filesystem::path(s_AssetPath) / path);
            }
            ImGui::EndDragDropTarget();
        }

        bool isViewportHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
        Entity selectedEntity = m_SceneGraphPanel->GetSelectedEntity();
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        float windowWidth = (float)ImGui::GetWindowWidth();
        float windowHeight = (float)ImGui::GetWindowHeight();
        ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

        // Editor Camera
        const glm::mat4& cameraProjection = m_Camera.GetProjectionMatrix();
        glm::mat4 cameraView = m_Camera.GetViewMatrix();


        if (selectedEntity)
        {
            // Entity transform
            auto& tc = selectedEntity.GetComponent<TransformComponent>();
            glm::mat4 transform = tc.GetTransform();

            // Snapping
            bool snap = Input::IsKeyPressed(Key::CGR_KEY_LEFT_CONTROL);
            float snapValue = 0.5f; // Snap to 0.5m for translation/rotation
            // Snap to 10 degrees for rotation
            if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
                snapValue = 10.0f;

            float snapValues[] = { snapValue, snapValue, snapValue };
            ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
                nullptr, snap ? snapValues : nullptr);

            if (ImGuizmo::IsOver() && m_GizmoType != -1)
            {
                glm::vec3 translation{ 0.0f }, rotation{ 0.0f }, scale{ 1.0f };
                Math::DecomposeTransform(transform, translation, rotation, scale);

                glm::vec3 deltaRotation = rotation - tc.Rotation;
                tc.Translation = translation;
                tc.Rotation += deltaRotation;
                tc.Scale = scale;
            }
        }

        ImGui::Begin("World Settings");
        auto& pos = m_Camera.GetPosition();
        auto txt = std::format("Camera position x:{}, y:{}, z:{}", pos.x, pos.y, pos.z);
        ImGui::Text(txt.c_str());
        if (ImGui::ColorEdit4("Clear color", glm::value_ptr(m_ClearColor)))
            RenderCommand::SetClearColor(m_ClearColor);
        if (ImGui::ColorEdit3("Ambient Light", glm::value_ptr(m_Renderer->m_AmbientLight)))
            m_Renderer->m_WorldSettings->SetData(sizeof(glm::vec4), sizeof(glm::vec3), glm::value_ptr(m_Renderer->m_AmbientLight));
        if (ImGui::DragFloat3("Light Position", glm::value_ptr(m_Renderer->m_LightPosition)))
            m_Renderer->m_WorldSettings->SetData(sizeof(glm::vec4) * 2, sizeof(glm::vec3), glm::value_ptr(m_Renderer->m_LightPosition));
        
        m_ReflectionSystem->ReflectClass("OpenGLSkybox");
        m_ReflectionSystem->ReflectClass("Camera");
        //m_Camera.UpdateProjectionMatrix();

        ImGui::End();

        m_ContentBrowserPanel->OnImGuiRender();
        m_SceneGraphPanel->OnImGuiRender();

        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::End();
    }

	void EditorLayer::OnEvent(Event& e)
	{
        m_Camera.OnEvent(e);

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(CGR_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(CGR_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
	}

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        // Shortcuts
        if (e.GetRepeatCount() > 0)
            return false;

        bool control = Input::IsKeyPressed(Key::CGR_KEY_LEFT_CONTROL) || Input::IsKeyPressed(Key::CGR_KEY_RIGHT_CONTROL);
        bool shift = Input::IsKeyPressed(Key::CGR_KEY_LEFT_SHIFT) || Input::IsKeyPressed(Key::CGR_KEY_RIGHT_SHIFT);

        switch (e.GetKey())
        {
        case Key::CGR_KEY_N:
        {
            if (control)
                NewScene();
            break;
        }
        case Key::CGR_KEY_O:
        {
            if (control)
                OpenScene();
            break;
        }
        case Key::CGR_KEY_S:
        {
            if (control && shift)
                SaveSceneAs();
            break;
        }
        default:
            break;
        }

        if (!m_Camera.IsMouseMoving())
        {
            switch (e.GetKey())
            {
                // Gizmo
            case Key::CGR_KEY_Q:
            {
                m_GizmoType = -1;
                break;
            }
            case Key::CGR_KEY_W:
            {
                m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
                break;
            }
            case Key::CGR_KEY_E:
            {
                m_GizmoType = ImGuizmo::OPERATION::ROTATE;
                break;
            }
            case Key::CGR_KEY_R:
            {
                m_GizmoType = ImGuizmo::OPERATION::SCALE;
                break;
            }
            default:
                break;
            }
        }

        return false;
    }

    bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        if (e.GetMouseButton() == Mouse::CGR_BUTTON_LEFT)
        {
            if (m_ViewportHovered && !ImGuizmo::IsOver() && !ImGuizmo::IsUsing() && !Input::IsKeyPressed(Key::CGR_KEY_LEFT_ALT))
                m_SceneGraphPanel->SetSelectedEntity(m_HoveredEntity);
        }

        return false;
    }

    void EditorLayer::NewScene()
    {
        m_ActiveScene = CreateRef<Scene>();
        m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
        m_SceneGraphPanel->SetContext(m_ActiveScene);
    }

    void EditorLayer::OpenScene()
    {
        std::string filepath = FileDialogs::OpenFile("CGR Scene (*.cgr)\0*.cgr\0");
        if (!filepath.empty())
        {
            OpenScene(filepath);
        }
    }

    void EditorLayer::OpenScene(const std::filesystem::path& path)
    {
        m_ActiveScene = CreateRef<Scene>();
        m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
        m_SceneGraphPanel->SetContext(m_ActiveScene);

        SceneSerializer serializer(m_ActiveScene);
        serializer.Deserialize(path.string());
    }

    void EditorLayer::SaveSceneAs()
    {
        std::string filepath = FileDialogs::SaveFile("CGR Scene (*.cgr)\0*.cgr\0");
        if (!filepath.empty())
        {
            SceneSerializer serializer(m_ActiveScene);
            serializer.Serialize(filepath);
        }
    }

}
