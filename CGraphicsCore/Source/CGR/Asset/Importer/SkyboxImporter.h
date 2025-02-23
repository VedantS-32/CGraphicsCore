#pragma once

#include "CGR/Renderer/EnvironmentMap.h"
#include "CGR/Asset/AssetMetadata.h"

namespace Cgr
{
	class SkyboxImporter
	{
	public:
		static Ref<Skybox> ImportSkybox(AssetHandle handle, const AssetMetadata& metadata);

	private:
		static Ref<Skybox> LoadSkybox(const std::filesystem::path& filePath);
	};
}