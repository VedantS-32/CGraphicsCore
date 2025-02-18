#include "CGRpch.h"
#include "EnvironmentMap.h"

#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLEnvironmentMap.h"

namespace Cgr
{
    Ref<CubeMap> CubeMap::Create()
    {
        switch (RendererAPI::GetAPI())
        {
        case Cgr::API::None:
            CGR_CORE_ASSERT("No Graphics API selected");
            break;
        case Cgr::API::OpenGL:
            return CreateRef<OpenGLCubeMap>();
            break;
        default:
            break;
        }

        return nullptr;
    }
}
