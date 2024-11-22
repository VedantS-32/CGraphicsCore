#include <CGR.h>

#include <CGR/Core/Entrypoint.h>

#include "EditorLayer.h"

namespace Cgr
{
	class CGraphicsSandbox : public Application
	{
	public:
		CGraphicsSandbox() : Application("CGraphicsSandbox", 900, 720)
		{
			PushLayer(new EditorLayer("Editor"));
		}
	};

	Application* CreateApplication()
	{
		CGR_INFO("Created Sandbox Application");
		return new CGraphicsSandbox();
	}
}