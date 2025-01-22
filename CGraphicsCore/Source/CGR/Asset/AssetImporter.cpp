#include "CGRpch.h"
#include "AssetImporter.h"

#include "Importer/TextureImporter.h"
#include "Importer/ShaderImporter.h"
#include "Importer/MaterialImporter.h"
#include "Importer/ModelImporter.h"

namespace Cgr
{
    namespace Utils
    {
        static std::string GetStringFromTypeEnum(AssetType type)
        {
            switch (type)
            {
            case Cgr::AssetType::None:
                return "None";
                break;
            case Cgr::AssetType::Texture2D:
                return "Texture2D";
                break;
            case Cgr::AssetType::SkyBox:
                return "SkyBox";
                break;
            case Cgr::AssetType::Shader:
                return "Shader";
                break;
            case Cgr::AssetType::Material:
                return "Material";
                break;
            case Cgr::AssetType::Model:
                return "Model";
                break;
            default:
                return "None";
                break;
            }

            return "None";
        }
    }

    using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>;
    static std::map<AssetType, AssetImportFunction> s_AssetImportFunctions =
    {
        { AssetType::Texture2D, TextureImporter::ImportTexture2D },
        { AssetType::Shader, ShaderImporter::ImportShader },
        { AssetType::Material, MaterialImporter::ImportMaterial },
        { AssetType::Model, ModelImporter::ImportModel }
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
