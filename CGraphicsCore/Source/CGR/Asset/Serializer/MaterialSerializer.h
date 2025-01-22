#pragma once

#include "CGR/Renderer/Material.h"

namespace Cgr
{
	class MaterialSerializer
	{
	public:
		void Serialize(Ref<Material> material);
	};
}