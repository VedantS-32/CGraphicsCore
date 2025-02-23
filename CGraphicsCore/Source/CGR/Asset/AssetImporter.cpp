#include "CGRpch.h"
#include "AssetImporter.h"

#include "AssetManager.h"
#include "Importer/TextureImporter.h"
#include "Importer/ShaderImporter.h"
#include "Importer/MaterialImporter.h"
#include "Importer/ModelImporter.h"
#include "Importer/SkyboxImporter.h"

namespace Cgr
{
    using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>;
    static std::map<AssetType, AssetImportFunction> s_AssetImportFunctions =
    {
        { AssetType::Texture2D, TextureImporter::ImportTexture2D },
        { AssetType::Shader, ShaderImporter::ImportShader },
        { AssetType::Material, MaterialImporter::ImportMaterial },
        { AssetType::Model, ModelImporter::ImportModel },
        { AssetType::Skybox, SkyboxImporter::ImportSkybox }
    };

    Ref<Asset> AssetImporter::ImportAsset(AssetHandle handle, const AssetMetadata& metadata)
    {
        if (!s_AssetImportFunctions.contains(metadata.Type))
        {
            CGR_CORE_ERROR("Cannot import asset of type: {0}", Utils::GetStringFromTypeEnum(metadata.Type));
            return nullptr;
        }
        return s_AssetImportFunctions[metadata.Type](handle, metadata);
    }
}
