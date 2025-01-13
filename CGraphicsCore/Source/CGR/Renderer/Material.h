#pragma once

#include "CGR/Core/Core.h"
#include "Buffer.h"
#include "Shader.h"
#include "ShaderDataType.h"
#include "Texture.h"

#include <glm/glm.hpp>

namespace Cgr
{
	class CGR_API Material
	{
	public:
		Material(Ref<Shader> shader);

		void SetShader(Ref<Shader> shader);

		template<typename T, typename ... Args>
		void AddVariable(Args&& ... args)
		{
			auto variable = CreateRef<T>(std::forward<Args>(args)...);
			m_ShaderVariables.emplace_back(variable);
		}

		const std::vector<Ref<ShaderVariable>>& GetAllVariables() { return m_ShaderVariables; }
		const std::unordered_map<std::string, Ref<Texture>>& GetAllTextures() { return m_Textures; }

		void AddTexture(const std::string& name, Ref<Texture> texture) { m_Textures[name] = texture; }

		Ref<Shader> GetShader() const { return m_Shader; }

		void UpdateSSBOParameters(Ref<ShaderStorageBuffer> SSBO);

	public:
		static Ref<Material> Create(Ref<Shader> shader);

	private:
		Ref<Shader> m_Shader;
		std::unordered_map<std::string, Ref<Texture>> m_Textures;
		std::string m_Name = "Default";

		// Material paramters stored in RAM
		std::vector<Ref<ShaderVariable>> m_ShaderVariables;
	};
}