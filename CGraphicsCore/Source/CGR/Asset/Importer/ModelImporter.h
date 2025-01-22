#pragma once

#include "CGR/Renderer/Model.h"
#include "CGR/Asset/AssetMetadata.h"

namespace Cgr
{
	class ModelImporter
	{
	public:
		static Ref<Model> ImportModel(AssetHandle handle, const AssetMetadata& metadata);

	private:
		static Ref<Model> LoadModel(const std::filesystem::path& filePath);
	};
}