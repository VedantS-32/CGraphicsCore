#pragma once

#include "CGR/Renderer/EnvironmentMap.h"

namespace Cgr
{
	class CGR_API SkyboxSerializer
	{
	public:
		SkyboxSerializer(Ref<Skybox> Skybox);

		void Serialize(const std::filesystem::path& path);
		void Deserialize(const std::filesystem::path& path);

	private:
		Ref<Skybox> m_Skybox;
	};
}