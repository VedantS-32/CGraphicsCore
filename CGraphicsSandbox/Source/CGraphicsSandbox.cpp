#include <CGR.h>

#include <CGR/Core/Entrypoint.h>

namespace Cgr
{
	class CGraphicsSandbox : public Application
	{
	public:
		CGraphicsSandbox() : Application("CGraphicsSandbox", 1080, 720)
		{}
	};

	Application* CreateApplication()
	{
		CGR_INFO("Created Sandbox Application");
		return new CGraphicsSandbox();
	}
}