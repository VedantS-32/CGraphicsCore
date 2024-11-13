#pragma once
#include <cstdint>

namespace Cgr
{
	using MouseCode = uint16_t;

	namespace Mouse
	{
		enum : MouseCode
		{
			// From glfw3.h
			CGR_BUTTON0 = 0,
			CGR_BUTTON1 = 1,
			CGR_BUTTON2 = 2,
			CGR_BUTTON3 = 3,
			CGR_BUTTON4 = 4,
			CGR_BUTTON5 = 5,
			CGR_BUTTON6 = 6,
			CGR_BUTTON7 = 7,

			CGR_BUTTON_LAST = CGR_BUTTON7,
			CGR_BUTTON_LEFT = CGR_BUTTON0,
			CGR_BUTTON_RIGHT = CGR_BUTTON1,
			CGR_BUTTON_MIDDLE = CGR_BUTTON2
		};
	}
}