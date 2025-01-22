#pragma once

#include "CGR/Core/Core.h"
#include "CGR/Core/UUID.h"

namespace Cgr
{
	using AssetHandle = UUID;

	enum class CGR_API AssetType : uint16_t
	{
		None = 0,
		Texture2D,
		SkyBox,
		Shader,
		Material,
		Model
	};

	class CGR_API Asset
	{
	public:
		AssetHandle Handle;
		virtual AssetType GetType() const = 0;
	};
}