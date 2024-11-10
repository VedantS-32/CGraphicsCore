#include "CGRpch.h"

#include "CrossWindow.h"
#include "Platform/OpenGL/OpenGLContext.h"

#include "stb_image.h"

namespace Cgr
{
    static bool s_GLFWInitialized = false;

    static void GLFWErrorCallback(int error, const char* description)
    {
        CGR_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    Scope<Window> Window::Create(const WindowProps& props)
    {
        return CreateScope<CrossWindow>(props);
    }

    CrossWindow::CrossWindow(const WindowProps& props)
        : m_WindowProps(props)
    {
        Init(props);
    }

    CrossWindow::~CrossWindow()
    {
        Shutdown();
    }

    void CrossWindow::Init(const WindowProps& props)
    {
        if (!s_GLFWInitialized)
        {
            // TODO: glfwTerminate on system shutdown
            int success = glfwInit();
            CGR_CORE_ASSERT(success, "Could not initialize GLFW!");
            glfwSetErrorCallback(GLFWErrorCallback);

            s_GLFWInitialized = true;
        }

        m_WindowHandle = glfwCreateWindow((int)props.Width, (int)props.Height, props.Title.c_str(), nullptr, nullptr);

        GLFWimage images[1]{};
        images[0].pixels = stbi_load("Content/Icon/CStell.png", &images[0].width, &images[0].height, 0, 4);
        glfwSetWindowIcon(m_WindowHandle, 1, images);
        stbi_image_free(images[0].pixels);

        m_RendererContext = new OpenGLContext(m_WindowHandle);
        m_RendererContext->Init();

        glfwSetWindowUserPointer(m_WindowHandle, &m_WindowProps);
        SetVSync(true);
    }

    void CrossWindow::OnUpdate()
    {
        m_RendererContext->SwapBuffer();
    }

    void CrossWindow::SetVSync(bool enabled)
    {
        if (enabled) { glfwSwapInterval(1); }
        else { glfwSwapInterval(0); }

        m_WindowProps.VSync = enabled;
    }

    bool CrossWindow::IsVSync() const
    {
        return m_WindowProps.VSync;
    }

    void CrossWindow::Shutdown()
    {
        glfwDestroyWindow(m_WindowHandle);
    }
}
