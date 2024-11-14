#include "CGRpch.h"
#include "LayerStack.h"
#include "Layer.h"

namespace Cgr
{
	LayerStack::LayerStack()
	{
	}

	LayerStack::~LayerStack()
	{
		for (auto* layer : m_LayerStack)
			delete layer;
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		m_LayerStack.emplace(m_LayerStack.begin() + m_InsertIndex, layer);
		m_InsertIndex++;
	}

	void LayerStack::PushOverlay(Layer* layer)
	{
		m_LayerStack.emplace_back(layer);
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto it = std::find(m_LayerStack.begin(), m_LayerStack.begin() + m_InsertIndex, layer);
		if (it != m_LayerStack.end())
		{
			m_LayerStack.erase(it);
			m_InsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer* layer)
	{
		auto it = std::find(m_LayerStack.begin() + m_InsertIndex, m_LayerStack.end(), layer);
		if (it != m_LayerStack.end())
		{
			m_LayerStack.erase(it);
		}
	}
}
