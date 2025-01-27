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
        MaterialSerializer serializer(material);
        serializer.Deserialize(filePath);
        CGR_CORE_TRACE("Imported Material asset, path: {0}", filePath.string());
        return material;
    }
}
