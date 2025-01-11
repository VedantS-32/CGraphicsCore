#pragma once

#include "CGR/Core/Core.h"
#include "Timestep.h"

namespace Cgr
{
	class Event;

	class CGR_API Layer
	{
	public:
		Layer(const std::string& layerName)
			: m_LayerName(layerName) {}

		virtual ~Layer() {}

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnUIRender() {}
		virtual void OnEvent(Event& e) {}

		const std::string& GetLayerName() { return m_LayerName; }

	protected:
		std::string m_LayerName;

	};
}