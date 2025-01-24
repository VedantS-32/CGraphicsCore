#pragma once

#include "CGR/Renderer/Material.h"

namespace Cgr
{
	class CGR_API MaterialSerializer
	{
	public:
		MaterialSerializer(Ref<Material> material);

		void Serialize(const std::filesystem::path& path);
		void Deserialize(const std::filesystem::path& path);

	private:
		Ref<Material> m_Material;
	};
}