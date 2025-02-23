#include "CGRpch.h"

#include "Application.h"
#include "CGR/Event/Event.h"
#include "CGR/Event/ApplicationEvent.h"
#include "CGR/Core/Layer.h"
#include "CGR/Core/LayerStack.h"
#include "CGR/Renderer/RenderCommand.h"
#include "Window.h"

#include <GLFW/glfw3.h>

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

        RenderCommand::Init();

        m_EventManager = EventManager(&m_LayerStack);

        m_ReflectionSystem = new Reflection;
        m_ImGuiLayer = new ImGuiLayer;
        m_AssetManager = new AssetManager;
        m_Renderer = new Renderer;
        m_AssetManager->LoadDefaultAssets();
        PushOverlay(m_ImGuiLayer);
    }

    Application::~Application()
    {
        delete m_ImGuiLayer;
        delete m_AssetManager;
        delete m_Renderer;
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
        m_EventManager.DispatchGlobalEvents(e);

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowClosedEvent>(CGR_BIND_EVENT_FN(OnWindowClose));

        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            (*it)->OnEvent(e);
            if (e.Handled)
            {
                break;
            }
        }
    }

    void Application::Run()
    {
        m_IsRunning = true;

        float time = (float)glfwGetTime();
        Timestep timestep = time - m_LastFrameTime;
        m_LastFrameTime = time;

        while (m_IsRunning)
        {
            for (Layer* layer : m_LayerStack)
                layer->OnUpdate(timestep);

            m_ImGuiLayer->Begin();
            for (Layer* layer : m_LayerStack)
                layer->OnUIRender();
            m_ImGuiLayer->End();

            m_Window->OnUpdate();
        }
    }

    void Application::Close()
    {
        m_IsRunning = false;
    }

    Reflection* Application::GetReflectionSystem()
    {
        return m_ReflectionSystem;
    }

    Application& Application::Get()
    {
        return *s_Application;
    }

    void Application::OnWindowClose(WindowClosedEvent& e)
    {
        CGR_CORE_INFO("{0} window closed", m_Window->GetTitle());
        m_IsRunning = false;
    }
}
