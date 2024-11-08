#include <CGR.h>

#include <CGR/Core/Entrypoint.h>

namespace Cgr
{
	class CGraphicsSandbox : public Application
	{
	public:
		CGraphicsSandbox()
		{
			CGR_TRACE("Creating Client Application...");
		}
	};

	Application* Cgr::CreateApplication()
	{
		CGR_INFO("Hellow Computer Graphics!");
		return new CGraphicsSandbox();
	}
}