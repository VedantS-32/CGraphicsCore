#pragma once

#include "CGR/Core/Core.h"
#include <unordered_map>
#include <string>

namespace Cgr
{
	enum class CGR_API ShaderType
	{
		None = 0, Vertex, Fragment, Geometry, Compute, Tessellation
	};

	class CGR_API Shader
	{
	public:
		virtual ~Shader() {}

		// Automatically parses source, preprocesses and compiles the shader
		virtual void PrepareShader() = 0;

		virtual std::string ParseShader(const std::string& shaderPath) = 0;
		virtual std::unordered_map<ShaderType, std::string> PreProcess(const std::string& source) = 0;
		virtual void CompileShaders(const std::unordered_map<ShaderType, std::string>& shaderSources) = 0;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

	public:
		static Ref<Shader> Create(const std::string& shaderPath);
	};
}