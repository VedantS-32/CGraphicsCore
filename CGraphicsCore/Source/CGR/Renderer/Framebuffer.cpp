#include "CGRpch.h"
#include "Framebuffer.h"

#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"


namespace Cgr
{
    Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
    {
		switch (RendererAPI::GetAPI())
		{
		case Cgr::API::None:
			CGR_CORE_ASSERT(false, "No Graphics API selected");
			break;
		case Cgr::API::OpenGL:
			return CreateRef<OpenGLFramebuffer>(spec);
			break;
		default:
			break;
		}

		return nullptr;
    }
}
