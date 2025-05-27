#pragma once

#include "CGR/Scene/Scene.h"

namespace Cgr
{
	class CGR_API SceneSerializer
	{
	public:
		SceneSerializer(Scene* Scene);

		void Serialize(const std::filesystem::path& path);
		void Deserialize(const std::filesystem::path& path);

	private:
		Scene* m_Scene;
	};
}