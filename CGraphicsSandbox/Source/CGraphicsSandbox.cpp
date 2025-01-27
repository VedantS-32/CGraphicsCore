#include "CGR.h"
#include "CGR/Core/Entrypoint.h"

#include "EditorLayer.h"

#include <imgui.h>

namespace Cgr
{
	class CGraphicsSandbox : public Application
	{
	public:
		CGraphicsSandbox() : Application("CGraphicsSandbox", 1080, 720)
		{
			ImGui::SetCurrentContext(Application::Get().GetUILayer()->GetImGuiContext());
			//std::cout << "ImGui context : " << ImGui::GetCurrentContext() << std::endl;
			PushLayer(new EditorLayer("Editor"));
		}
	};

	Application* CreateApplication()
	{
		CGR_INFO("Created Sandbox Application");
		return new CGraphicsSandbox();
	}
}