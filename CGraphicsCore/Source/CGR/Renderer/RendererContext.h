#pragma once

#include "CGR/Core/Core.h"

namespace Cgr
{
	class CGR_API RendererContext
	{
	public:
		virtual void Init() = 0;
		virtual void SwapBuffer() = 0;
	};
}