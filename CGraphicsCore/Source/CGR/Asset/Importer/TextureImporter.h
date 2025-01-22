#pragma once

#include "CGR/Renderer/Texture.h"
#include "CGR/Asset/AssetMetadata.h"

namespace Cgr
{
	class TextureImporter
	{
	public:
		static Ref<Texture2D> ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata);

	private:
		static Ref<Texture2D> LoadTexture2D(const std::filesystem::path& filePath);
	};
}