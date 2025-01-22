#include "CGRpch.h"
#include "ImGuiLayer.h"

#include "CGR/Core/Application.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace Cgr
{
	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{
	}

	void ImGuiLayer::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		m_ImGuiContext = ImGui::CreateContext();
		ImGui::SetCurrentContext(m_ImGuiContext);
		//std::cout << "ImGui context core: " << ImGui::GetCurrentContext() << std::endl;
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		io.Fonts->AddFontFromFileTTF("Content/Font/Karla/Static/Karla-Bold.ttf", 17.0f);
		io.FontDefault = io.Fonts->AddFontFromFileTTF("Content/Font/Karla/Static/Karla-Regular.ttf", 17.0f);

		// Setup Dear ImGui style
		//ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// CStellObservatory Theme
		SetDarkTheme();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

	}

	void ImGuiLayer::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
	
	void ImGuiLayer::OnUpdate(Timestep ts)
	{
	}

	void ImGuiLayer::OnUIRender()
	{
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
	}

	ImGuiContext* ImGuiLayer::GetImGuiContext()
	{
		return m_ImGuiContext;
	}

	void ImGuiLayer::Begin()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void ImGuiLayer::SetDarkTheme()
	{
		// From https://github.com/Patitotective/ImThemes
// Fork of Moonlight style from ImThemes
		ImGuiStyle& style = ImGui::GetStyle();

		style.Alpha = 0.86f;
		style.DisabledAlpha = 1.0f;
		style.WindowPadding = ImVec2(12.0f, 12.0f);
		style.WindowRounding = 11.5f;
		style.WindowBorderSize = 1.0f;
		style.WindowMinSize = ImVec2(20.0f, 20.0f);
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
		style.WindowMenuButtonPosition = ImGuiDir_Right;
		style.ChildRounding = 0.0f;
		style.ChildBorderSize = 0.0f;
		style.PopupRounding = 0.0f;
		style.PopupBorderSize = 1.0f;
		style.FramePadding = ImVec2(10.0f, 1.0f);
		style.FrameRounding = 5.0f;
		style.FrameBorderSize = 0.0f;
		style.ItemSpacing = ImVec2(3.0f, 3.0f);
		style.ItemInnerSpacing = ImVec2(5.0f, 5.0f);
		style.CellPadding = ImVec2(12.0f, 8.0f);
		style.IndentSpacing = 2.0f;
		style.ColumnsMinSpacing = 5.0f;
		style.ScrollbarSize = 19.5f;
		style.ScrollbarRounding = 2.0f;
		style.GrabMinSize = 8.0f;
		style.GrabRounding = 20.0f;
		style.TabRounding = 0.0f;
		style.TabBorderSize = 0.0f;
		style.TabMinWidthForCloseButton = 0.0f;
		style.ColorButtonPosition = ImGuiDir_Right;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

		// Fork of Purple Comfy style from ImThemes
		style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(1.0f, 1.0f, 1.0f, 0.36f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
		style.Colors[ImGuiCol_Border] = ImVec4(1.0f, 0.3f, 0.53f, 0.65f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.42f, 0.57f, 0.55f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.87f, 0.49f, 0.61f, 0.54f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.0f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.25f, 0.25f, 0.25f, 0.0f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.0f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.23f, 0.23f, 0.23f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.29f, 0.29f, 0.29f, 1.0f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.32f, 0.49f, 1.0f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.88f, 0.34f, 0.45f, 1.0f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.34f, 0.5f, 1.0f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.91f, 0.33f, 0.47f, 1.0f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.95f, 0.6f, 0.78f, 1.0f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.35f, 0.57f, 1.0f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.43f, 0.25f, 0.33f, 1.0f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.9f, 0.34f, 0.47f, 1.0f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.3f, 0.48f, 1.0f);
		style.Colors[ImGuiCol_Separator] = ImVec4(1.0f, 0.3f, 0.46f, 0.53f);
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(1.0f, 0.3f, 0.46f, 0.53f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(1.0f, 0.3f, 0.46f, 0.53f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.95f, 0.38f, 0.57f, 0.0f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 0.3f, 0.42f, 1.0f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 0.3f, 0.46f, 1.0f);
		style.Colors[ImGuiCol_Tab] = ImVec4(0.43f, 0.25f, 0.33f, 1.0f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(0.89f, 0.34f, 0.47f, 1.0f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(1.0f, 0.3f, 0.48f, 1.0f);
		style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0f, 0.45f, 1.0f, 0.0f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.25f, 0.42f, 0.0f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.29f, 0.29f, 0.29f, 1.0f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.88f, 0.46f, 0.6f, 1.0f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.51f, 0.71f, 0.77f, 1.0f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.73f, 0.69f, 0.88f, 0.54f);
		style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.18f, 0.18f, 0.2f, 1.0f);
		style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(1.0f, 0.3f, 0.4f, 1.0f);
		style.Colors[ImGuiCol_TableBorderLight] = ImVec4(1.0f, 0.3f, 0.4f, 1.0f);
		style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.03f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.0f, 0.3f, 0.46f, 1.0f);
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.89f);
		style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.69f);
		style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.34f);

	}
}
