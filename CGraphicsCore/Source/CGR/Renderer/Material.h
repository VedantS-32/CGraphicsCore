#pragma once

#include "CGR/Core/Core.h"
#include "CGR/Asset/Asset.h"
#include "Buffer.h"
#include "Shader.h"
#include "ShaderDataType.h"
#include "Texture.h"

#include <map>
#include <glm/glm.hpp>
#include <vector>

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

		bool HasVariable(const std::string& name) const
		{
			for (auto& variable : m_ShaderVariables)
			{
				if (variable->GetName() == name)
					return true;
			}

			return false;
		}
		
		std::vector<Ref<ShaderVariable>>& GetAllVariables(){ return m_ShaderVariables; }
		std::vector<Ref<Texture>>& GetAllTextures() { return m_Textures; }

		void AddTexture(Ref<Texture> texture)
		{
			m_Textures.emplace_back(texture);
		}

		Ref<Shader> GetShader() const { return m_Shader; }

		void UpdateSSBOParameters(Ref<ShaderStorageBuffer> SSBO);

		static AssetType GetStaticType() { return AssetType::Material; }
		virtual AssetType GetType() const { return GetStaticType(); }

	public:
		static Ref<Material> Create(Ref<Shader> shader);
		static Ref<Material> Create();

	private:
		Ref<Shader> m_Shader;
		std::vector<Ref<Texture>> m_Textures;
		std::string m_Name = "Default";

		// Material paramters stored in RAM
		std::vector<Ref<ShaderVariable>> m_ShaderVariables;
	};
}