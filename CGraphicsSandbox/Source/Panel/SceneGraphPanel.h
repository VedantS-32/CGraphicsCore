#pragma once

#include "CGR.h"
#include "ContentBrowserPanel.h"

namespace Cgr
{
	class SceneGraphPanel
	{
	public:
		SceneGraphPanel() = default;
		SceneGraphPanel(const Ref<Scene>& context, ContentBrowserPanel* contentBrowserPanel);

		void SetContext(const Ref<Scene>& context);

		void OnImGuiRender();

		Entity GetSelectedEntity() const { return m_SelectionContext; }
		void SetSelectedEntity(Entity entity);

	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);

	private:
		AssetManager* m_AssetManager;
		Renderer* m_Renderer;
		ContentBrowserPanel* m_ContentBrowserPanel;
		Ref<Scene> m_Context;
		Ref<Texture2D> m_DefaultTexture;
		Entity m_SelectionContext;
		Entity m_PreviousContext;
	};
}