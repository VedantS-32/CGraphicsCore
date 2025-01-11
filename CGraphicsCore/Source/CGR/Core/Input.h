#pragma once

#include "Cgr/Core/Core.h"
#include "Cgr/Core/KeyCode.h"
#include "Cgr/Core/MouseButtonCode.h"
#include "Cgr/Core/InputMode.h"

namespace Cgr
{
	class CGR_API Input
	{
	public:
		static bool IsKeyPressed(const KeyCode keycode);

		static bool IsMouseButtonPressed(const MouseCode button);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();

		static void SetInputMode(Mode mode, Cursor value);
	};
}