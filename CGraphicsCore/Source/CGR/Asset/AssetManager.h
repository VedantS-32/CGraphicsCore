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
			case Cgr::AssetType::Skybox:
				return "Skybox";
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

	class CGR_API AssetManager
	{
	public:
		AssetManager() = default;
		void LoadDefaultAssets();

		template<typename T>
		// Don't create model, shader or texture asset with this function
		Ref<T> CreateAsset(const std::filesystem::path pathToSave)
		{
			Ref<T> asset = CreateRef<T>();
			AssetHandle handle; // Generates new handle
			AssetMetadata metadata;

			metadata.Path = pathToSave;
			metadata.Type = T::GetStaticType();

			asset->Handle = handle;
			asset->Name = pathToSave.filename().string();

			m_LoadedAssets[handle] = asset;
			m_AssetRegistry[handle] = metadata;

			CGR_CORE_TRACE("Created new asset of type: {0}", Utils::GetStringFromTypeEnum(T::GetStaticType()))

			return std::static_pointer_cast<T>(asset);
		}

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

		AssetMetadata& GetAssetMetadata(AssetHandle handle);
		const std::filesystem::path& GetFilePath(AssetHandle handle);
		void SetFilePath(AssetHandle handle, const std::filesystem::path& path);

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