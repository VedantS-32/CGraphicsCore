#pragma once

#include "CGR/Core/Layer.h"

namespace Cgr
{
	class ImGuiLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate() override;
		virtual void OnUIRender() override;
		virtual void OnEvent(Event& e) override;
	};
}