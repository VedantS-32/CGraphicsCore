#pragma once
#include <cstdint>

namespace Cgr
{
	using Mode = uint32_t;
	using Cursor = uint32_t;

	namespace InputMode
	{
		enum : Mode
		{
			// From glfw3.h
			CGR_CURSOR = 0x00033001,
			CGR_STICKY_KEYS = 0x00033002,
			CGR_STICKY_MOUSE_BUTTONS = 0x00033003,
			CGR_LOCK_KEY_MODS = 0x00033004,
			CGR_RAW_MOUSE_MOTION = 0x00033005
		};
	}

	namespace CursorMode
	{
		enum : Cursor
		{
			CGR_CURSOR_NORMAL = 0x00034001,
			CGR_CURSOR_HIDDEN = 0x00034002,
			CGR_CURSOR_DISABLED = 0x00034003,
			CGR_CURSOR_CAPTURED = 0x00034004
		};
	}
}