#include "CGRpch.h"
#include "Application.h"

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
    }

    void Application::Run()
    {
        m_IsRunning = true;

        while (m_IsRunning)
        {
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
}
