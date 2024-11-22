#include "CGRpch.h"
#include "VertexArray.h"

#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Cgr
{
    Ref<VertexArray> VertexArray::Create(const BufferLayout& bufferLayout)
    {
        switch (RendererAPI::GetAPI())
        {
        case Cgr::API::None:
            CGR_CORE_ASSERT(false, "No Graphics API selected");
            break;
        case Cgr::API::OpenGL:
            return CreateRef<OpenGLVertexArray>(bufferLayout);
            break;
        default:
            break;
        }

        return Ref<VertexArray>();
    }
}
