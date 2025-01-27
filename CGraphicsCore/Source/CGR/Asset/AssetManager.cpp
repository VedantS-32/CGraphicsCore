#include "CGRpch.h"
#include "AssetManager.h"

#include "AssetImporter.h"

namespace Cgr
{
    namespace Utils
    {
        static std::map<std::filesystem::path, AssetType> s_AssetExtensionMap =
        {
            { ".png", AssetType::Texture2D },
            { ".jpg", AssetType::Texture2D },
            { ".jpeg", AssetType::Texture2D },
            { ".glsl", AssetType::Shader },
            { ".csmat", AssetType::Material},
            { ".obj", AssetType::Model },
            { ".fbx", AssetType::Model },
            { ".csmesh", AssetType::Model }
        };

        static AssetType GetAssetTypeFromExtension(std::filesystem::path extension)
        {
            if (!s_AssetExtensionMap.contains(extension))
            {
                CGR_CORE_WARN("Invalid file type: {0}", extension.string());
                return AssetType::None;
            }

            return s_AssetExtensionMap[extension];
        }
    }

    void AssetManager::LoadDefaultAssets()
    {
        std::vector<std::string> defaultAssets =
        {
            "Content/Texture/UVChecker.png",
            "Content/Icon/CStell.png",
            "Content/Shader/Default.glsl",
            "Content/Shader/Default.csmat",
            "Content/Model/Cube.obj",
            "Content/Model/Cube.csmesh"
        };

        for (auto& path : defaultAssets)
        {
            AssetHandle handle = ImportAsset(path);
            if (handle)
                m_DefaultAssets[GetAssetType(handle)] = handle;
            else
                CGR_CORE_ERROR("Couldn't import default asset!");
        }
    }

    AssetType AssetManager::GetAssetType(AssetHandle handle)
    {
        if (!IsAssetHandleValid(handle))
            return AssetType::None;
        return m_AssetRegistry[handle].Type;
    }

    bool AssetManager::IsAssetHandleValid(AssetHandle handle)
    {
        CGR_CORE_ASSERT(handle != 0, "Asset handle is not valid");
        return handle != 0 && m_AssetRegistry.contains(handle);
    }

    bool AssetManager::IsAssetLoaded(AssetHandle handle)
    {
        return m_LoadedAssets.contains(handle);
    }

    AssetHandle AssetManager::ImportAsset(const std::filesystem::path& filePath)
    {
        auto type = Utils::GetAssetTypeFromExtension(filePath.extension());
        AssetHandle handle; //Generates new handle
        for (auto& [assetHandle, metadata] : m_AssetRegistry)
        {
            if (metadata.Path == filePath && metadata.Type == type)
            {
                handle = assetHandle;
                return handle;
            }
        }

        CGR_CORE_ASSERT(type != AssetType::None);
        AssetMetadata metadata(type, filePath);

        Ref<Asset> asset = AssetImporter::ImportAsset(handle, metadata);
        if(asset)
        {
            asset->Handle = handle;
            asset->Name = filePath.stem().string();
            m_LoadedAssets[handle] = asset;
            m_AssetRegistry[handle] = metadata;
        }
        return handle;
    }

    AssetMetadata& AssetManager::GetAssetMetadata(AssetHandle handle)
    {
        return m_AssetRegistry[handle];
    }

    const std::filesystem::path& AssetManager::GetFilePath(AssetHandle handle)
    {
        return m_AssetRegistry[handle].Path;
    }

    void AssetManager::SetFilePath(AssetHandle handle, const std::filesystem::path& path)
    {
        m_AssetRegistry[handle].Path = path;
    }

    const AssetRegistry& AssetManager::GetAssetRegistry()
    {
        return m_AssetRegistry;
    }

    const AssetHandle AssetManager::GetAssetHandleFromRegistry(const std::filesystem::path& filePath)
    {
        for (auto& [assetHandle, metadata] : m_AssetRegistry)
        {
            if (metadata.Path == filePath)
            {
                return assetHandle;
            }
        }
        return 0;
    }

    const AssetMap& AssetManager::GetLoadedAssets()
    {
        return m_LoadedAssets;
    }

    const DefaultAssets& AssetManager::GetDefaultAssetHandles()
    {
        return m_DefaultAssets;
    }
}
