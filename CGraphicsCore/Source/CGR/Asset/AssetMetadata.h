#pragma once

#include "CGR/Core/Core.h"
#include "Asset.h"

#include <filesystem>

namespace Cgr
{
	struct CGR_API BinaryInfo
	{
		const void* Data = nullptr;
		size_t Size = 0;
	};

	class CGR_API AssetMetadata
	{
	public:
		AssetMetadata() = default;
		AssetMetadata(AssetType type, std::filesystem::path path)
			: Type(type), Path(path) {}

		AssetMetadata(AssetType type, BinaryInfo info)
			: Type(type), BinInfo(info) {}

		AssetType Type = AssetType::None;
		std::filesystem::path Path;

		BinaryInfo BinInfo;

		operator bool() const { return Type != AssetType::None; }
	};
}