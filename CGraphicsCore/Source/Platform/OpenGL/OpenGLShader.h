#pragma once

#include "CGR/Renderer/Shader.h"
#include "CGR/Renderer/ShaderDataTypes.h"

#include <glad/glad.h>

namespace Cgr
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& shaderPath);

		virtual void PrepareShader() override;

		virtual std::string ParseShader(const std::string & shaderPath) override;
		virtual std::unordered_map<ShaderType, std::string> PreProcess(const std::string& source) override;
		virtual void CompileShaders(const std::unordered_map<ShaderType, std::string>& shaderSources) override;

		virtual void Bind() override;
		virtual void Unbind() override;

	public:
		static ShaderType ShaderTypeFromString(const std::string& type);
		static GLenum ToOpenGLShaderType(ShaderType type);
		static GLenum ToOpenGLDataType(ShaderDataType type);

	private:
		std::string m_ShaderPath;

	private:
		uint32_t m_RendererID;
	};
}