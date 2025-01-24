#pragma once

#include "CGR/Core/Core.h"
#include "CGR/Asset/Asset.h"
#include "Buffer.h"
#include "Shader.h"
#include "ShaderDataType.h"
#include "Texture.h"

#include <glm/glm.hpp>

namespace Cgr
{
	class CGR_API Material : public Asset
	{
	public:
		Material(Ref<Shader> shader);
		Material();

		void SetShader(Ref<Shader> shader);

		template<typename T, typename ... Args>
		void AddVariable(Args&& ... args)
		{
			auto variable = CreateRef<T>(std::forward<Args>(args)...);
			m_ShaderVariables.emplace_back(variable);
		}

		std::vector<Ref<ShaderVariable>>& GetAllVariables() { return m_ShaderVariables; }
		std::unordered_map<std::string, Ref<Texture>>& GetAllTextures() { return m_Textures; }

		void AddTexture(const std::string& name, Ref<Texture> texture) { m_Textures[name] = texture; }

		Ref<Shader> GetShader() const { return m_Shader; }

		void UpdateSSBOParameters(Ref<ShaderStorageBuffer> SSBO);

		static AssetType GetStaticType() { return AssetType::Material; }
		virtual AssetType GetType() const { return GetStaticType(); }

	public:
		static Ref<Material> Create(Ref<Shader> shader);
		static Ref<Material> Create();

	private:
		Ref<Shader> m_Shader;
		std::unordered_map<std::string, Ref<Texture>> m_Textures;
		std::string m_Name = "Default";

		// Material paramters stored in RAM
		std::vector<Ref<ShaderVariable>> m_ShaderVariables;
	};
}