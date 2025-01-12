#pragma once

#include "CGR/Core/Core.h"
#include "Shader.h"
#include "ShaderDataType.h"

#include <glm/glm.hpp>
#include "Buffer.h"

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

		std::vector<Ref<ShaderVariable>>& GetAllVariables()
		{
			return m_ShaderVariables;
		}

		Ref<Shader> GetShader() const { return m_Shader; }
		Ref<UniformBuffer> GetUniformBuffer() const { return m_UniformBuffer; }

		void UpdateSSBOParameters(Ref<ShaderStorageBuffer> SSBO);

	public:
		static Ref<Material> Create(Ref<Shader> shader);

	private:
		Ref<Shader> m_Shader;
		Ref<UniformBuffer> m_UniformBuffer;
		std::string m_Name = "Default";

		// Material paramters stored in RAM
		std::vector<Ref<ShaderVariable>> m_ShaderVariables;
	};
}