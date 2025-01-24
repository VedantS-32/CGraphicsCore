#include "CGRpch.h"
#include "MaterialImporter.h"

#include "CGR/Asset/Serializer/MaterialSerializer.h"

namespace Cgr
{
    Ref<Material> MaterialImporter::ImportMaterial(AssetHandle handle, const AssetMetadata& metadata)
    {
        return LoadMaterial(metadata.Path);
    }

    Ref<Material> MaterialImporter::LoadMaterial(const std::filesystem::path& filePath)
    {
        auto material = Material::Create();
        CGR_CORE_INFO("Imported Material asset, path: {0}", filePath.string());
        MaterialSerializer serializer(material);
        serializer.Deserialize(filePath);
        return material;
    }
}
