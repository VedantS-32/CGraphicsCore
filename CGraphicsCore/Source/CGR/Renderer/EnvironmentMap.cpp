#include "CGRpch.h"
#include "EnvironmentMap.h"

#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLEnvironmentMap.h"

namespace Cgr
{
    Ref<Skybox> Skybox::Create()
    {
        switch (RendererAPI::GetAPI())
        {
        case Cgr::API::None:
            CGR_CORE_ASSERT("No Graphics API selected");
            break;
        case Cgr::API::OpenGL:
            return CreateRef<OpenGLSkybox>();
            break;
        default:
            break;
        }

        return nullptr;
    }
}
