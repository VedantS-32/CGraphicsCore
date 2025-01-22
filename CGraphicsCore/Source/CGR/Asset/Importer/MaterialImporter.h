#pragma once

#include "CGR/Core/Core.h"
#include "CGR/Asset/AssetMetadata.h"
#include "CGR/Renderer/Material.h"

namespace Cgr
{
	class MaterialImporter
	{
	public:
		static Ref<Material> ImportMaterial(AssetHandle handle, const AssetMetadata& metadata);

	private:
		static Ref<Material> LoadMaterial(const std::filesystem::path& filePath);
	};
}