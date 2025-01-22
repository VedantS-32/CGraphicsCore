#pragma once

#include "CGR/Core/Layer.h"

#include <imgui_internal.h>

namespace Cgr
{
	class CGR_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnUIRender() override;
		virtual void OnEvent(Event& e) override;

		ImGuiContext* GetImGuiContext();

	public:
		void Begin();
		void End();

		void SetDarkTheme();

	private:
		ImGuiContext* m_ImGuiContext;
		bool m_BlockEvents = true;
	};
}