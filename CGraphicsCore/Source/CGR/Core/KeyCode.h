#pragma once
#include <cstdint>

namespace Cgr
{
	using KeyCode = uint16_t;

	namespace Key
	{
		enum : uint16_t
		{
			CGR_KEY_SPACE = 32,
			CGR_KEY_APOSTROPHE = 39,	/* ' */
			CGR_KEY_COMMA = 44,	/* , */
			CGR_KEY_MINUS = 45,	/* - */
			CGR_KEY_PERIOD = 46,	/* . */
			CGR_KEY_SLASH = 47,	/* / */
			CGR_KEY_0 = 48,
			CGR_KEY_1 = 49,
			CGR_KEY_2 = 50,
			CGR_KEY_3 = 51,
			CGR_KEY_4 = 52,
			CGR_KEY_5 = 53,
			CGR_KEY_6 = 54,
			CGR_KEY_7 = 55,
			CGR_KEY_8 = 56,
			CGR_KEY_9 = 57,
			CGR_KEY_SEMICOLON = 59,	/* ; */
			CGR_KEY_EQUAL = 61,  /* = */
			CGR_KEY_A = 65,
			CGR_KEY_B = 66,
			CGR_KEY_C = 67,
			CGR_KEY_D = 68,
			CGR_KEY_E = 69,
			CGR_KEY_F = 70,
			CGR_KEY_G = 71,
			CGR_KEY_H = 72,
			CGR_KEY_I = 73,
			CGR_KEY_J = 74,
			CGR_KEY_K = 75,
			CGR_KEY_L = 76,
			CGR_KEY_M = 77,
			CGR_KEY_N = 78,
			CGR_KEY_O = 79,
			CGR_KEY_P = 80,
			CGR_KEY_Q = 81,
			CGR_KEY_R = 82,
			CGR_KEY_S = 83,
			CGR_KEY_T = 84,
			CGR_KEY_U = 85,
			CGR_KEY_V = 86,
			CGR_KEY_W = 87,
			CGR_KEY_X = 88,
			CGR_KEY_Y = 89,
			CGR_KEY_Z = 90,
			CGR_KEY_LEFT_BRACKET = 91,	/* [ */
			CGR_KEY_BACKSLASH = 92,	/* \ */
			CGR_KEY_RIGHT_BRACKET = 93,	/* ] */
			CGR_KEY_GRAVE_ACCENT = 96,	/* ` */
			CGR_KEY_WORLD_1 = 161,	/* non-US #1 */
			CGR_KEY_WORLD_2 = 162,	/* non-US #2 */

			/*function keys*/
			CGR_KEY_ESCAPE = 256,
			CGR_KEY_ENTER = 257,
			CGR_KEY_TAB = 258,
			CGR_KEY_BACKSPACE = 259,
			CGR_KEY_INSERT = 260,
			CGR_KEY_DELETE = 261,
			CGR_KEY_RIGHT = 262,
			CGR_KEY_LEFT = 263,
			CGR_KEY_DOWN = 264,
			CGR_KEY_UP = 265,
			CGR_KEY_PAGE_UP = 266,
			CGR_KEY_PAGE_DOWN = 267,
			CGR_KEY_HOME = 268,
			CGR_KEY_END = 269,
			CGR_KEY_CAPS_LOCK = 280,
			CGR_KEY_SCROLL_LOCK = 281,
			CGR_KEY_NUM_LOCK = 282,
			CGR_KEY_PRINT_SCREEN = 283,
			CGR_KEY_PAUSE = 284,
			CGR_KEY_F1 = 290,
			CGR_KEY_F2 = 291,
			CGR_KEY_F3 = 292,
			CGR_KEY_F4 = 293,
			CGR_KEY_F5 = 294,
			CGR_KEY_F6 = 295,
			CGR_KEY_F7 = 296,
			CGR_KEY_F8 = 297,
			CGR_KEY_F9 = 298,
			CGR_KEY_F10 = 299,
			CGR_KEY_F11 = 300,
			CGR_KEY_F12 = 301,
			CGR_KEY_F13 = 302,
			CGR_KEY_F14 = 303,
			CGR_KEY_F15 = 304,
			CGR_KEY_F16 = 305,
			CGR_KEY_F17 = 306,
			CGR_KEY_F18 = 307,
			CGR_KEY_F19 = 308,
			CGR_KEY_F20 = 309,
			CGR_KEY_F21 = 310,
			CGR_KEY_F22 = 311,
			CGR_KEY_F23 = 312,
			CGR_KEY_F24 = 313,
			CGR_KEY_F25 = 314,
			CGR_KEY_KP_0 = 320,
			CGR_KEY_KP_1 = 321,
			CGR_KEY_KP_2 = 322,
			CGR_KEY_KP_3 = 323,
			CGR_KEY_KP_4 = 324,
			CGR_KEY_KP_5 = 325,
			CGR_KEY_KP_6 = 326,
			CGR_KEY_KP_7 = 327,
			CGR_KEY_KP_8 = 328,
			CGR_KEY_KP_9 = 329,
			CGR_KEY_KP_DECIMAL = 330,
			CGR_KEY_KP_DIVIDE = 331,
			CGR_KEY_KP_MULTIPLY = 332,
			CGR_KEY_KP_SUBTRACT = 333,
			CGR_KEY_KP_ADD = 334,
			CGR_KEY_KP_ENTER = 335,
			CGR_KEY_KP_EQUAL = 336,
			CGR_KEY_LEFT_SHIFT = 340,
			CGR_KEY_LEFT_CONTROL = 341,
			CGR_KEY_LEFT_ALT = 342,
			CGR_KEY_LEFT_SUPER = 343,
			CGR_KEY_RIGHT_SHIFT = 344,
			CGR_KEY_RIGHT_CONTROL = 345,
			CGR_KEY_RIGHT_ALT = 346,
			CGR_KEY_RIGHT_SUPER = 347,
			CGR_KEY_MENU = 348
		};
	}
}