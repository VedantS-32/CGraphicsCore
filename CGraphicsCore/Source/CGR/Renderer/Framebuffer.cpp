#include "CGRpch.h"
#include "Framebuffer.h"

#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"


namespace Cgr
{
    Ref<Framebuffer> Framebuffer::Create()
    {
		switch (RendererAPI::GetAPI())
		{
		case Cgr::API::None:
			CGR_CORE_ASSERT(false, "No Graphics API selected");
			break;
		case Cgr::API::OpenGL:
			return CreateRef<OpenGLFramebuffer>();
			break;
		default:
			break;
		}

		return nullptr;
    }
}
