#include "CGRpch.h"
#include "TextureImporter.h"

#include "CGR/Utils/Utils.h"


#include <stb_image.h>

namespace Cgr
{
	Ref<Texture2D> TextureImporter::ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata)
    {
		switch (metadata.Type)
		{
		case Cgr::AssetType::Texture2D:
			return LoadTexture2D(metadata.Path);
			break;
		case Cgr::AssetType::BinaryTexture2D:
			return LoadBinaryTexture2D(metadata);
			break;
		default:
			return nullptr;
			break;
		}

		CGR_CORE_ASSERT(false, ("Unsupported asset type for texture import: {0}", Utils::GetStringFromTypeEnum(metadata.Type)));
		return nullptr;
    }

    Ref<Texture2D> TextureImporter::LoadTexture2D(const std::filesystem::path& filePath)
    {
		int width, height, channels;
		std::string name;
		stbi_uc* data = nullptr;

		stbi_set_flip_vertically_on_load(1);
		{
			auto path = filePath.string();
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
			name = Utils::ExtractNameFromFilepath(path);
		}
		if (!data)
		{
			data = stbi_load("Content/Texture/UVChecker.png", &width, &height, &channels, 0);
			name = "DefaultTexture";
		}

		TextureSpecification spec;
		spec.Width = width;
		spec.Height = height;

		if (channels == 4)
		{
			spec.Format = ImageFormat::RGBA8;
		}
		else if (channels == 3)
		{
			spec.Format = ImageFormat::RGB8;
		}

		auto texture = Texture2D::Create(spec, data);
		texture->SetName(name);
		stbi_image_free(data);

		CGR_CORE_TRACE("Imported Texture2D asset, path: {0}", filePath.string());

		return texture;
    }

	Ref<Texture2D> TextureImporter::LoadBinaryTexture2D(const AssetMetadata& metadata)
	{
		auto name = metadata.Path.string();

		TextureSpecification spec;
		spec.Width = 40;
		spec.Height = 32;
		spec.Format = ImageFormat::RGBA8;

		auto texture = Texture2D::Create(spec, metadata.BinInfo.Data);
		texture->SetName(name);

		CGR_CORE_TRACE("Imported BinaryTexture2D asset: {0}", name);

		return texture;
	}
}
