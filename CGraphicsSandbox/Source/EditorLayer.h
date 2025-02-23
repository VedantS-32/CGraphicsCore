#pragma once

#include <CGR.h>

#include "Panel/ContentBrowserPanel.h"
#include "Panel/SceneGraphPanel.h"

#include <glm/glm.hpp>

namespace Cgr
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer(const std::string& layerName);
		~EditorLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override { CGR_TRACE("Detached {0} layer", m_LayerName); }
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnUIRender() override;
		virtual void OnEvent(Event& e) override;

	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveSceneAs();

	private:
		Ref<Framebuffer> m_Framebuffer;
		Camera m_Camera;

		AssetManager* m_AssetManager;
		Renderer* m_Renderer;
		Reflection* m_ReflectionSystem;

		Ref<Scene> m_ActiveScene;
		Entity m_HoveredEntity;

		glm::vec4 m_ClearColor;
		glm::vec2 m_ViewportSize;
		glm::vec2 m_ViewportBounds[2];
		bool m_ViewportFocused;
		bool m_ViewportHovered;
		
		int m_GizmoType = -1;

		int m_PreviousEntity = -1;
		int m_CurrentEntity = -1;

		// Panels
		ContentBrowserPanel* m_ContentBrowserPanel;
		SceneGraphPanel* m_SceneGraphPanel;
	};
}