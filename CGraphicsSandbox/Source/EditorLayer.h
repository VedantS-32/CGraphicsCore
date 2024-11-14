#pragma once

#include<CGR.h>

namespace Cgr
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer(const std::string& layerName)
			: Layer(layerName) {}

		virtual void OnAttach() { CGR_TRACE("Attached {0} layer", m_LayerName); }
		virtual void OnDetach() { CGR_TRACE("Detached {0} layer", m_LayerName); }
		virtual void OnUpdate();
		virtual void OnUIRender();
		virtual void OnEvent(const Event& e);
	};
}