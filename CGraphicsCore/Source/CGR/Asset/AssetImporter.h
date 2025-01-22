#pragma once

#include "CGR/Core/Core.h"
#include "Asset.h"
#include "AssetMetadata.h"

namespace Cgr
{
	class CGR_API AssetImporter
	{
	public:
		static Ref<Asset> ImportAsset(AssetHandle handle, const AssetMetadata& metadata);
	};
}