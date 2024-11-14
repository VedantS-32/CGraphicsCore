#include "CGRpch.h"

#include "Application.h"
#include "CGR/Event/Event.h"
#include "CGR/Core/Layer.h"
#include "CGR/Core/LayerStack.h"
#include "Window.h"

namespace Cgr
{
    Application* Application::s_Application = nullptr;

    Application::Application(std::string title, uint32_t width, uint32_t height)
    {
        CGR_CORE_INFO("Starting CGraphicsCore");

        s_Application = this;

        m_Window = Window::Create(WindowProps(title, width, height));
        m_Window->SetVSync(true);

        m_Window->SetEventCallback(CGR_BIND_EVENT_FN(OnEvent));

        m_EventManager.SetLayerStack(&m_LayerStack);

        m_EventManager.AddListener<MouseScrolledEvent>(CGR_BIND_EVENT_FN(Hello));
    }

    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* layer)
    {
        m_LayerStack.PushOverlay(layer);
        layer->OnAttach();
    }

    void Application::OnEvent(Event& e)
    {
        m_EventManager.Dispatch(e);
    }

    void Application::Run()
    {
        m_IsRunning = true;

        while (m_IsRunning)
        {
            for (Layer* layer : m_LayerStack)
                layer->OnUpdate();

            m_Window->OnUpdate();
        }
    }

    void Application::Close()
    {
        m_IsRunning = false;
    }

    Application& Application::Get()
    {
        return *s_Application;
    }

    void Application::Hello(const Event& e)
    {
        CGR_CORE_TRACE("Hellow, {0}", e.ToString());
    }
}
