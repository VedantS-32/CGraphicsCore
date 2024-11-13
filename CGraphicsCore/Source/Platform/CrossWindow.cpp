#include "CGRpch.h"

#include "CrossWindow.h"
#include "Platform/OpenGL/OpenGLContext.h"

#include "CGR/Event/ApplicationEvent.h"
#include "CGR/Event/KeyEvent.h"
#include "CGR/Event/MouseEvent.h"

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
    {
        m_WindowData.Title = props.Title;
        m_WindowData.Width = props.Width;
        m_WindowData.Height = props.Height;

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

        glfwSetWindowUserPointer(m_WindowHandle, &m_WindowData);
        SetVSync(true);

		// Set GLFW Callbacks
		glfwSetWindowSizeCallback(m_WindowHandle, [](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.Width = width;
				data.Height = height;

				WindowResizedEvent event(width, height);
				data.EventCallback(event);
			}
		);

		glfwSetWindowCloseCallback(m_WindowHandle, [](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowClosedEvent event;
				data.EventCallback(event);
			}
		);

		glfwSetWindowPosCallback(m_WindowHandle, [](GLFWwindow* window, int xPos, int yPos)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowMovedEvent event(xPos, yPos);
				data.EventCallback(event);
			}
		);

		glfwSetKeyCallback(m_WindowHandle, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
					case GLFW_PRESS:
					{
						KeyPressedEvent event(key, 0);
						data.EventCallback(event);
						break;
					}
					case GLFW_RELEASE:
					{
						KeyReleasedEvent event(key);
						data.EventCallback(event);
						break;
					}
					case GLFW_REPEAT:
					{
						KeyPressedEvent event(key, 1);
						data.EventCallback(event);
						break;
					}
				}
			}
		);

		glfwSetCharCallback(m_WindowHandle, [](GLFWwindow* window, unsigned int keycode)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				KeyTypedEvent event(keycode);
				data.EventCallback(event);
			});

		glfwSetMouseButtonCallback(m_WindowHandle, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
					case GLFW_PRESS:
					{
						MouseButtonPressedEvent event(button);
						data.EventCallback(event);
						break;
					}
					case GLFW_RELEASE:
					{
						MouseButtonReleasedEvent event(button);
						data.EventCallback(event);
						break;
					}
				}
			}
		);

		glfwSetScrollCallback(m_WindowHandle, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
				data.EventCallback(event);
			}
		);

		glfwSetCursorPosCallback(m_WindowHandle, [](GLFWwindow* window, double xPos, double yPos)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
				data.EventCallback(event);
			}
		);
    }

    void CrossWindow::OnUpdate()
    {
        m_RendererContext->SwapBuffer();
		glfwPollEvents();
    }

    void CrossWindow::SetVSync(bool enabled)
    {
        if (enabled) { glfwSwapInterval(1); }
        else { glfwSwapInterval(0); }

        m_WindowData.VSync = enabled;
    }

    bool CrossWindow::IsVSync() const
    {
        return m_WindowData.VSync;
    }

    void CrossWindow::Shutdown()
    {
        glfwDestroyWindow(m_WindowHandle);
    }
}
