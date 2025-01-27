#pragma once

#include "CGR/Renderer/Shader.h"
#include "CGR/Asset/AssetMetadata.h"

namespace Cgr
{
	class ShaderImporter
	{
	public:
		static Ref<Shader> ImportShader(AssetHandle handle, const AssetMetadata& metadata);

	private:
		static Ref<Shader> LoadShader(const std::filesystem::path& filePath);

		static std::string ParseFile(const std::string& filePath);
		static std::unordered_map<ShaderType, std::string> PreProcess(const std::string& source);
	};
}