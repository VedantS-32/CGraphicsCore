#include "CGRpch.h"

#include "Texture.h"
#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Cgr
{
    Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
    {
        switch (RendererAPI::GetAPI())
        {
        case Cgr::API::None:
            CGR_CORE_ASSERT("No Graphics API selected");
            break;
        case Cgr::API::OpenGL:
            return CreateRef<OpenGLTexture2D>(width, height);
            break;
        default:
            break;
        }

        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const TextureSpecification& spec, const void* data)
    {
        switch (RendererAPI::GetAPI())
        {
        case Cgr::API::None:
            CGR_CORE_ASSERT("No Graphics API selected");
            break;
        case Cgr::API::OpenGL:
            return CreateRef<OpenGLTexture2D>(spec, data);
            break;
        default:
            break;
        }

        return nullptr;
    }
}
