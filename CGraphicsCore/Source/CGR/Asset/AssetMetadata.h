#pragma once

#include "CGR/Core/Core.h"
#include "Asset.h"

#include <filesystem>

namespace Cgr
{
	class CGR_API AssetMetadata
	{
	public:
		AssetMetadata() = default;
		AssetMetadata(AssetType type, std::filesystem::path path)
			: Type(type), Path(path) {}

		AssetType Type = AssetType::None;
		std::filesystem::path Path;

		operator bool() const { return Type != AssetType::None; }
	};
}