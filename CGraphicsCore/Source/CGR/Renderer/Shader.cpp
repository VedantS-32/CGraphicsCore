#include "CGRpch.h"
#include "Shader.h"

#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Cgr
{
    Ref<Shader> Shader::Create(const std::string& shaderPath)
    {
        switch (RendererAPI::GetAPI())
        {
        case Cgr::API::None:
            CGR_CORE_ASSERT("No Graphics API selected");
            break;
        case Cgr::API::OpenGL:
            return CreateRef<OpenGLShader>(shaderPath);
            break;
        default:
            break;
        }

        return nullptr;
    }
}