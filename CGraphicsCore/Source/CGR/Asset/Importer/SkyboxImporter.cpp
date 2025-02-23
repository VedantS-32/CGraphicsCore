#include "CGRpch.h"
#include "SkyboxImporter.h"

#include "CGR/Utils/Utils.h"
#include "CGR/Asset/Serializer/SkyboxSerializer.h"

#include <stb_image.h>

namespace Cgr
{
	Ref<Skybox> SkyboxImporter::ImportSkybox(AssetHandle handle, const AssetMetadata& metadata)
    {
        return LoadSkybox(metadata.Path);
    }

    Ref<Skybox> SkyboxImporter::LoadSkybox(const std::filesystem::path& filePath)
    {
        auto skybox = Skybox::Create();
        SkyboxSerializer serializer(skybox);
        serializer.Deserialize(filePath);
		CGR_CORE_TRACE("Imported Skybox asset, path: {0}", filePath.string());
		return skybox;
    }
}
