#pragma once

#include "CGR/Scene/Scene.h"

namespace Cgr
{
	class CGR_API SceneSerializer
	{
	public:
		SceneSerializer(Ref<Scene> Scene);

		void Serialize(const std::filesystem::path& path);
		void Deserialize(const std::filesystem::path& path);

	private:
		Ref<Scene> m_Scene;
	};
}