#pragma once

#include "CGR/Core/Core.h"
#include "CGR/Renderer/ShaderDataType.h"
#include "CGR/Asset/Asset.h"

#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

namespace Cgr
{
	class Material;
	class ShaderStorageBuffer;

	enum class CGR_API ShaderType
	{
		None = 0, Vertex, Fragment, Geometry, Compute, Tessellation
	};

	class CGR_API Shader : public Asset
	{
	public:
		virtual ~Shader() {}

		// Automatically parses source, preprocesses and compiles the shader
		virtual void PrepareShader() = 0;

		virtual std::string ParseShader(const std::string& shaderPath) = 0;
		virtual std::unordered_map<ShaderType, std::string> PreProcess(const std::string& source) = 0;
		virtual void CompileShaders(const std::unordered_map<ShaderType, std::string>& shaderSources) = 0;
		virtual const std::string& GetPath() = 0;

		virtual void Set1i(const std::string& name, int value) = 0;
		virtual void Set1i(const std::string& name, int value, uint32_t offset) = 0;
		virtual void Set2i(const std::string& name, const glm::uvec2& value) = 0;
		virtual void Set3i(const std::string& name, const glm::uvec3& value) = 0;
		virtual void Set4i(const std::string& name, const glm::uvec4& value) = 0;
		virtual void Set1iArray(const std::string& name, int* value, uint32_t count) = 0;
		virtual void Set1f(const std::string& name, float value) = 0;
		virtual void Set2f(const std::string& name, const glm::vec2& value) = 0;
		virtual void Set3f(const std::string& name, const glm::vec3& value) = 0;
		virtual void Set4f(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat3f(const std::string& name, const glm::mat3& matrix) = 0;
		virtual void SetMat4f(const std::string& name, const glm::mat4& matrix) = 0;

		virtual uint32_t GetUniformLocation(const std::string& name) = 0;
		virtual const std::string& GetName() = 0;

		virtual void ExtractSSBOParameters(Material* material) = 0;
		virtual void UpdateSSBOParameters(Material* material, Ref<ShaderStorageBuffer> SSBO) = 0;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual uint32_t GetRendererID() const = 0;

		static AssetType GetStaticType() { return AssetType::Shader; }
		virtual AssetType GetType() const { return GetStaticType(); }

	public:
		static Ref<Shader> Create(const std::string& name, const std::string& shaderPath);
		static Ref<Shader> Create(const std::unordered_map<ShaderType, std::string>& shaderSource);
	};

	class CGR_API ShaderLibrary
	{
	public:
		ShaderLibrary();

		void Add(const std::string& shaderPath);
		void Add(Ref<Shader> shader);
		void Add(const std::string& name, Ref<Shader> shader);
		bool Exists(const std::string& name);
		Ref<Shader> Get(const std::string& name);

	public:
		static Ref<ShaderLibrary> Create();

	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}