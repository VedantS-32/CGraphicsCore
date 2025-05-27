#pragma once

#include "CGR.h"

namespace Cgr
{
	class AddScriptPanel
	{
	public:
		static void OpenAddScriptPanel();
		static bool OnImGuiRender(std::string& scriptPath);
	};
}