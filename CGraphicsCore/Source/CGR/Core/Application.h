#pragma once

#include "Core.h"

namespace Cgr
{
	class CGR_API Application
	{
	public:
		Application();

		void Run();
	};

	CGR_API Application* CreateApplication();
}