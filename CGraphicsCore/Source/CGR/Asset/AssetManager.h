#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "AssetImporter.h"

#include <map>

namespace Cgr
{
	using AssetRegistry = std::map<AssetHandle, AssetMetadata>;
	using AssetMap = std::map<AssetHandle, Ref<Asset>>;
	using DefaultAssets = std::map<AssetType, AssetHandle>;

	class CGR_API AssetManager
	{
	public:
		AssetManager();
		void LoadDefaultAssets();

		template<typename T>
		Ref<T> GetAsset(AssetHandle handle)
		{
			if (!IsAssetHandleValid(handle))
				return nullptr;

			Ref<Asset> asset;
			if (IsAssetLoaded(handle))
			{
				asset = m_LoadedAssets.at(handle);
			}
			else
			{
				const AssetMetadata& metadata = GetAssetMetadata(handle);
				asset = AssetImporter::ImportAsset(handle, metadata);
				if (!asset)
				{
					// import failed
					CGR_CORE_ERROR("AssetManager::GetAsset - asset import failed!");
				}
				m_LoadedAssets[handle] = asset;
			}

			return std::static_pointer_cast<T>(asset);
		}

		AssetType GetAssetType(AssetHandle handle);
		bool IsAssetHandleValid(AssetHandle handle);
		bool IsAssetLoaded(AssetHandle handle);

		AssetHandle ImportAsset(const std::filesystem::path& filePath);

		const AssetMetadata& GetAssetMetadata(AssetHandle handle);
		const std::filesystem::path& GetFilePath(AssetHandle handle);

		const AssetRegistry& GetAssetRegistry();
		const AssetHandle GetAssetHandleFromRegistry(const std::filesystem::path& filePath);
		const AssetMap& GetLoadedAssets();
		AssetHandle GetDefaultAssetHandle(AssetType type) { return m_DefaultAssets[type]; }
		const DefaultAssets& GetDefaultAssetHandles();

	private:
		AssetRegistry m_AssetRegistry;
		AssetMap m_LoadedAssets;
		DefaultAssets m_DefaultAssets;
	};
}