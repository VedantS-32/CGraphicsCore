#pragma once

#include "CGR/Renderer/Buffer.h"
#include "CGR/Renderer/Shader.h"
#include "CGR/Renderer/ShaderDataType.h"

// Temp
#include "CGR/Renderer/Texture.h"

#include <glad/glad.h>

namespace Cgr
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& name, const std::string& shaderPath);
		OpenGLShader(const std::unordered_map<ShaderType, std::string>& shaderSources);

		virtual void PrepareShader() override;

		virtual std::string ParseShader(const std::string & shaderPath) override;
		virtual std::unordered_map<ShaderType, std::string> PreProcess(const std::string& source) override;
		virtual void CompileShaders(const std::unordered_map<ShaderType, std::string>& shaderSources) override;
		virtual const std::string& GetPath() override;

		virtual void Set1i(const std::string& name, int value) override;
		virtual void Set1i(const std::string& name, int value, uint32_t offset) override;
		virtual void Set2i(const std::string& name, const glm::uvec2& value) override;
		virtual void Set3i(const std::string& name, const glm::uvec3& value) override;
		virtual void Set4i(const std::string& name, const glm::uvec4& value) override;
		virtual void Set1iArray(const std::string& name, int* value, uint32_t count) override;
		virtual void Set1f(const std::string& name, float value) override;
		virtual void Set2f(const std::string& name, const glm::vec2& value) override;
		virtual void Set3f(const std::string& name, const glm::vec3& value) override;
		virtual void Set4f(const std::string& name, const glm::vec4& value) override;
		virtual void SetMat3f(const std::string& name, const glm::mat3& matrix) override;
		virtual void SetMat4f(const std::string& name, const glm::mat4& matrix) override;

		// Set uniforms
		void UploadUniform1i(const std::string& name, int value);
		void UploadUniform1i(const std::string& name, int value, uint32_t offset);
		void UploadUniform2i(const std::string& name, const glm::uvec2& value);
		void UploadUniform3i(const std::string& name, const glm::uvec3& value);
		void UploadUniform4i(const std::string& name, const glm::uvec4& value);
		void UploadUniform1iArray(const std::string& name, int* value, uint32_t count);

		void UploadUniform1f(const std::string& name, float value);
		void UploadUniform2f(const std::string& name, const glm::vec2& value);
		void UploadUniform3f(const std::string& name, const glm::vec3& value);
		void UploadUniform4f(const std::string& name, const glm::vec4& value);

		void UploadUniformMat3f(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4f(const std::string& name, const glm::mat4& matrix);

		virtual const std::string& GetName() override { return m_Name; }

		virtual void ExtractSSBOParameters(Material* material) override;
		virtual void UpdateSSBOParameters(Material* material, Ref<ShaderStorageBuffer> SSBO) override;

		virtual uint32_t GetUniformLocation(const std::string& name) override;

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual uint32_t GetRendererID() const override { return m_RendererID; }

	public:
		static ShaderType ShaderTypeFromString(const std::string& type);
		static GLenum ToOpenGLShaderType(ShaderType type);
		static GLenum ToOpenGLDataType(ShaderDataType type);

	private:
		std::string m_ShaderPath;
		std::string m_Name;
		std::unordered_map<std::string, int> m_UniformLocationCache;

	private:
		uint32_t m_RendererID;
	};
}