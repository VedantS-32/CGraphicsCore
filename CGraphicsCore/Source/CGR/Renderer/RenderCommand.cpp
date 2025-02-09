#include "CGRpch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLAPI.h"

namespace Cgr
{
    RendererAPI* RenderCommand::s_RendererAPI = new OpenGLAPI;

    void RenderCommand::Init()
    {
        s_RendererAPI->Init();
    }

    RendererAPI& Cgr::RenderCommand::GetRendererAPI()
    {
        return *s_RendererAPI;
    }
}