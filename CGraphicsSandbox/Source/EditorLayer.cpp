#include "EditorLayer.h"

#include "CGR.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

namespace Cgr
{
    extern const std::filesystem::path s_AssetPath;

    static float s_LogoSize = 48.0f;

    static ImVec2 s_LastWindowSize = { 0, 0 };

    static bool s_IsDragging = false;

    static void HandleWindowResize()
    {
        ImVec2 currentWindowSize = ImGui::GetWindowSize();

        if (currentWindowSize.x != s_LastWindowSize.x ||
            currentWindowSize.y != s_LastWindowSize.y)
        {

			//const_cast<Window&>(Application::Get().GetWindow()).SetWindowSize(
				//(int)currentWindowSize.x, (int)currentWindowSize.y);

            s_LastWindowSize = currentWindowSize;
        }
    }

    static void HandleWindowDragging()
    {
        static glm::ivec2 dragOffset = { 0, 0 };
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 mousePos = io.MousePos;

        // Get the current window (should be called within a window context)
        if (!ImGui::GetCurrentWindow()) return;

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        // Define draggable area (top 40 pixels of window)
        ImRect draggableArea(windowPos.x, windowPos.y, windowPos.x + windowSize.x, windowPos.y + 40.0f);

        // Check if mouse is in draggable area
        bool mouseInDraggableArea = draggableArea.Contains(mousePos);
        bool IsAnyItemHovered = ImGui::IsAnyItemHovered();
        bool IsAnyItemActive = ImGui::IsAnyItemActive();
        bool IsAnyPopupOpen = ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);

        // Only handle dragging if:
        // 1. Mouse is in draggable area
        // 2. No UI items are being interacted with
        // 3. No popups are open
        if (mouseInDraggableArea && !IsAnyItemHovered && !IsAnyItemActive && !IsAnyPopupOpen)
        {
            // Handle drag start
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                dragOffset = { (int)(mousePos.x - windowPos.x), (int)(mousePos.y - windowPos.y) };
                s_IsDragging = true;
            }
        }

        // Handle dragging (continue dragging even if mouse moves outside draggable area)
        if (s_IsDragging && ImGui::IsMouseDown(ImGuiMouseButton_Left) && !IsAnyPopupOpen)
        {
            glm::ivec2 newPos = { mousePos.x - dragOffset.x, mousePos.y - dragOffset.y };
            const_cast<Window&>(Application::Get().GetWindow()).SetWindowPosition(newPos.x, newPos.y);
        }
        else
        {
            s_IsDragging = false;
        }
    }

	EditorLayer::EditorLayer(const std::string& layerName)
		: Layer(layerName), m_ClearColor(0.5f, 0.5f, 0.5f, 1.0f), m_ViewportSize(1280, 720)
    {
        m_AssetManager = Application::Get().GetAssetManager();
        m_Renderer = Application::Get().GetRenderer();
        m_ReflectionSystem = Application::Get().GetReflectionSystem();

        m_ActiveScene = CreateRef<Scene>();
        auto entity = m_ActiveScene->CreateEntity("FirstEntity");

        auto handle = m_AssetManager->ImportAsset("Content/Model/CStellCube.csmesh");
        auto& model = entity.AddComponent<ModelComponent>();
        model.SetModel(m_AssetManager->GetAsset<Model>(handle));

        handle = m_AssetManager->ImportAsset("Content/Model/Pot.fbx");
        entity = m_ActiveScene->CreateEntity("SecondEntity");
        auto& model1 = entity.AddComponent<ModelComponent>();
        model1.SetModel(m_AssetManager->GetAsset<Model>(handle));
        auto& transform = entity.GetComponent<TransformComponent>();

        handle = m_AssetManager->ImportAsset("Content/Icon/CStell.png");
		m_Logo = m_AssetManager->GetAsset<Texture2D>(handle);

		m_CloseButton = m_AssetManager->GetAsset<Texture2D>(m_AssetManager->GetQuickAccessHandle("CloseButton"));
		m_MaximizeButton = m_AssetManager->GetAsset<Texture2D>(m_AssetManager->GetQuickAccessHandle("MaximizeButton"));
		m_MinimizeButton = m_AssetManager->GetAsset<Texture2D>(m_AssetManager->GetQuickAccessHandle("MinimizeButton"));
		m_PlayButton = m_AssetManager->GetAsset<Texture2D>(m_AssetManager->GetQuickAccessHandle("PlayButton"));
		m_StopButton = m_AssetManager->GetAsset<Texture2D>(m_AssetManager->GetQuickAccessHandle("StopButton"));

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

        m_ScriptEngine = Application::Get().GetScriptEngine();
        //m_ScriptEngine->LoadScript("Content/Script/build/hwl.hl");
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
        Model::ResetTriangleCount();
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
        static bool firstTime = true;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar;

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::Begin("##DockSpace", &dockSpaceOpen, window_flags);

        HandleWindowResize();

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 370.0f;

        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("##DockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

            // Create default layout on first run
            if (firstTime)
            {
                firstTime = false;

                // Clear any existing layout
                ImGui::DockBuilderRemoveNode(dockspace_id);
                ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

                // Split the dockspace into toolbar and main content 
                ImGuiID dock_id_toolbar = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Up, s_LogoSize / viewport->WorkSize.y, &dock_id_toolbar, &dockspace_id);

                // Split main area into center and right sidebar
                ImGuiID dock_id_center;
                ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, &dock_id_right, &dock_id_center);

                // Split center into editor and content browser
                ImGuiID dock_id_editor;
                ImGuiID dock_id_content = ImGui::DockBuilderSplitNode(dock_id_center, ImGuiDir_Down, 0.3f, &dock_id_content, &dock_id_editor);

                // Split right sidebar into scene graph and properties
                ImGuiID dock_id_scene;
                ImGuiID dock_id_properties = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Down, 0.7f, &dock_id_properties, &dock_id_scene);

                // Configure toolbar dock node to be fixed at top
                ImGui::DockBuilderGetNode(dock_id_toolbar)->LocalFlags |= ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoResizeY | ImGuiDockNodeFlags_NoDockingOverMe;

                // Dock windows to specific areas
                ImGui::DockBuilderDockWindow("Toolbar", dock_id_toolbar);
                ImGui::DockBuilderDockWindow("Viewport", dock_id_editor);
                ImGui::DockBuilderDockWindow("World Settings", dock_id_scene);
                ImGui::DockBuilderDockWindow("Scene Graph", dock_id_scene);
                ImGui::DockBuilderDockWindow("Properties", dock_id_properties);
                ImGui::DockBuilderDockWindow("Content Browser", dock_id_content);

                // Apply the layout
                ImGui::DockBuilderFinish(dockspace_id);
            }
        }

        ImGui::End();
        style.WindowMinSize.x = minWinSizeX;
        ImGui::PopStyleVar(2);

        ImGuiWindowFlags toolbar_flags =
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoResize;

        ImGui::Begin("Toolbar", nullptr, toolbar_flags);
        {
            // Logo and toolbar buttons
            ImGui::Image(m_Logo->GetRendererID(), { s_LogoSize, s_LogoSize }, ImVec2(0, 1), ImVec2(1, 0));
            ImGui::SameLine();

            // Toolbar buttons styling
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

            if (ImGui::Button("File", ImVec2(50.0f, 25.0f)))
            {
                ImGui::OpenPopup("FileMenu");
            }
            ImGui::SameLine();
            if (ImGui::Button("Edit", ImVec2(50.0f, 25.0f))) {}
            ImGui::SameLine();
            if (ImGui::Button("View", ImVec2(50.0f, 25.0f))) {}
            ImGui::SameLine();
            if (ImGui::Button("Tools", ImVec2(50.0f, 25.0f))) {}

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 6.0f, 6.0f });
            if (ImGui::BeginPopup("FileMenu"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N"))
                    NewScene();
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                    OpenScene();
                if (ImGui::MenuItem("Save", "Ctrl+Shift+S"))
                    SaveSceneAs();
                if (ImGui::MenuItem("Exit"))
                    Application::Get().Close();
                ImGui::EndPopup();
            }
            ImGui::PopStyleVar();

            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(2);
        }

        // Play button section centered in the titlebar
        ImGui::SameLine(ImGui::GetWindowWidth() * 0.5f - s_LogoSize);

        // Play/Stop button styling
        ImVec4 playButtonColor = m_IsRuntime ? ImVec4(0.7f, 0.2f, 0.2f, 1.0f) : ImVec4(0.2f, 0.7f, 0.3f, 1.0f);
        ImVec4 playButtonHoveredColor = m_IsRuntime ? ImVec4(0.8f, 0.3f, 0.3f, 1.0f) : ImVec4(0.3f, 0.8f, 0.4f, 1.0f);
        ImVec4 playButtonActiveColor = m_IsRuntime ? ImVec4(0.6f, 0.1f, 0.1f, 1.0f) : ImVec4(0.1f, 0.6f, 0.2f, 1.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, playButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, playButtonHoveredColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, playButtonActiveColor);

		ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 6.0f); // Center vertically

        if (ImGui::ImageButton("Play", m_IsRuntime ? m_StopButton->GetRendererID() : m_PlayButton->GetRendererID(), ImVec2(40.0f, 32.0f)))
        {
            if (!m_IsRuntime)
            {
                // Start runtime
                m_IsRuntime = true;
                m_ActiveScene->StartRuntime();
            }
            else
            {
                // Stop runtime
                m_IsRuntime = false;
                NewScene();
                m_ActiveScene->StopRuntime();
            }
        }

        ImGui::PopStyleColor(3); // For play button colors

        // Window controls on the right
        float windowControlsWidth = 120.0f;
        ImGui::SameLine(ImGui::GetWindowWidth() - windowControlsWidth);

        // Minimize, Maximize, Close buttons
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 1.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.63f, 0.82f, 1.0f));
        if (ImGui::ImageButton("Minimize", m_MinimizeButton->GetRendererID(), ImVec2(40.0f, 32.0f)))
        {
			const_cast<Window&>(Application::Get().GetWindow()).Minimize();
        }
        ImGui::SameLine();
        if (ImGui::ImageButton("Maximize", m_MaximizeButton->GetRendererID(), ImVec2(40.0f, 32.0f)))
        {
			const_cast<Window&>(Application::Get().GetWindow()).Maximize();
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.87f, 0.33f, 0.28f, 1.0f));
        if (ImGui::ImageButton("Close", m_CloseButton->GetRendererID(), ImVec2(40.0f, 32.0f)))
        {
            Application::Get().Close();
        }
        ImGui::PopStyleColor();

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2); // For window control colors

        // Enable window dragging for the titlebar
        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
        {
            HandleWindowDragging();
        }

        ImGui::End();

        // Editor Window (Main Viewport)
        ImGui::Begin("Viewport");
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.48f, 0.8f, 1.0f));

            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar();
            ImGui::Separator();

            // Calculate viewport bounds and size
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

            // Handle viewport resize
            if ((m_ViewportSize != *((glm::vec2*)&viewportSize)) && (viewportSize.x > 0 && viewportSize.y > 0))
            {
                m_Camera.SetViewportSize(viewportSize.x, viewportSize.y);
                m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            }

            // Render viewport
            uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
            ImGui::Image(textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

            // Drag and drop handling
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    auto path = static_cast<const char*>(payload->Data);
                    OpenScene(std::filesystem::path(s_AssetPath) / path);
                }
                ImGui::EndDragDropTarget();
            }

            // Gizmo handling
            Entity selectedEntity = m_SceneGraphPanel->GetSelectedEntity();

            if (selectedEntity && m_GizmoType != -1)
            {
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();
                ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y,
                    m_ViewportBounds[1].x - m_ViewportBounds[0].x,
                    m_ViewportBounds[1].y - m_ViewportBounds[0].y);

                const glm::mat4& cameraProjection = m_Camera.GetProjectionMatrix();
                glm::mat4 cameraView = m_Camera.GetViewMatrix();
                auto& tc = selectedEntity.GetComponent<TransformComponent>();
                glm::mat4 transform = tc.GetTransform();

                bool snap = Input::IsKeyPressed(Key::CGR_KEY_LEFT_CONTROL);
                float snapValue = (m_GizmoType == ImGuizmo::OPERATION::ROTATE) ? 10.0f : 0.5f;
                float snapValues[] = { snapValue, snapValue, snapValue };

                ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                    (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
                    nullptr, snap ? snapValues : nullptr);

                if (ImGuizmo::IsUsing())
                {
                    glm::vec3 translation, rotation, scale;
                    Math::DecomposeTransform(transform, translation, rotation, scale);
                    glm::vec3 deltaRotation = rotation - tc.Rotation;
                    tc.Translation = translation;
                    tc.Rotation += deltaRotation;
                    tc.Scale = scale;
                }
            }
        }
        ImGui::End();

		//ImGui::ShowDemoWindow();

        ImGui::Begin("World Settings");
        ImGui::Text(std::format("FPS: {:.2f}", ImGui::GetIO().Framerate).c_str());
        ImGui::Text(std::format("Triangle Count: {}", Model::GetTriangleCount()).c_str());
        if (ImGui::Button("Reload Script!"))
        {
            OnReloadButtonClicked();
        }
        if (ImGui::Button("Call Haxe Function!"))
        {
            OnCallHaxeFuncButtonClicked();
        }
        auto& pos = m_Camera.GetPosition();
        auto txt = std::format("Camera position x:{}, y:{}, z:{}", pos.x, pos.y, pos.z);
        ImGui::Text(txt.c_str());
        if (ImGui::ColorEdit4("Clear color", glm::value_ptr(m_ClearColor)))
            RenderCommand::SetClearColor(m_ClearColor);
        if (ImGui::ColorEdit3("Ambient Light", glm::value_ptr(m_Renderer->m_AmbientLight)))
            m_Renderer->m_WorldSettings->SetData(sizeof(glm::vec4), sizeof(glm::vec3), glm::value_ptr(m_Renderer->m_AmbientLight));
        if (ImGui::DragFloat3("Light Position", glm::value_ptr(m_Renderer->m_LightPosition)))
            m_Renderer->m_WorldSettings->SetData(sizeof(glm::vec4) * 2, sizeof(glm::vec3), glm::value_ptr(m_Renderer->m_LightPosition));
        
        m_ReflectionSystem->ReflectClass("Skybox");
        m_ReflectionSystem->ReflectClass("Camera");
        m_ReflectionSystem->ReflectClass("OpenGLTexture");

        ImGui::End();

        m_ContentBrowserPanel->OnImGuiRender();
        m_SceneGraphPanel->OnImGuiRender();
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

    void EditorLayer::OnReloadButtonClicked()
    {
        m_ScriptEngine->LoadScript("Content/Script/build/hwl.hl");
    }

    void EditorLayer::OnCallHaxeFuncButtonClicked()
    {
        m_ScriptEngine->CallVoidFunction("hwlHaxe");
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

        SceneSerializer serializer(m_ActiveScene.get());
        serializer.Deserialize(path.string());
    }

    void EditorLayer::SaveSceneAs()
    {
        std::string filepath = FileDialogs::SaveFile("CGR Scene (*.cgr)\0*.cgr\0");
        if (!filepath.empty())
        {
            SceneSerializer serializer(m_ActiveScene.get());
            serializer.Serialize(filepath);
        }
    }
}
