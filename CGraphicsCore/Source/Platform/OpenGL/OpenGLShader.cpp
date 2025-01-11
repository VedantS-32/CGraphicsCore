#include "CGRpch.h"
#include "OpenGLShader.h"
#include "CGR/Renderer/Material.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Cgr
{
	ShaderType OpenGLShader::ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return ShaderType::Vertex;
		if (type == "fragment" || type == "pixel")
			return ShaderType::Fragment;
		if (type == "geometry")
			return ShaderType::Geometry;
		if (type == "compute")
			return ShaderType::Compute;
		if (type == "tessellation")
			return ShaderType::Tessellation;

		CGR_CORE_ASSERT(false, "Unknown shader type");
		return ShaderType::None;
	}

	GLenum OpenGLShader::ToOpenGLShaderType(ShaderType type)
	{
		switch (type)
		{
		case Cgr::ShaderType::Vertex:
			return GL_VERTEX_SHADER;
			break;
		case Cgr::ShaderType::Fragment:
			return GL_FRAGMENT_SHADER;
			break;
		case Cgr::ShaderType::Geometry:
			return GL_GEOMETRY_SHADER;
			break;
		case Cgr::ShaderType::Compute:
			return GL_COMPUTE_SHADER;
			break;
		case Cgr::ShaderType::Tessellation:
			return GL_TESS_CONTROL_SHADER;
			break;
		default:
			break;
		}

		return GL_NONE;
	}

	GLenum OpenGLShader::ToOpenGLDataType(ShaderDataType type)
	{
		switch (type)
		{
		case Cgr::ShaderDataType::Float:	return GL_FLOAT; break;
		case Cgr::ShaderDataType::Float2:	return GL_FLOAT; break;
		case Cgr::ShaderDataType::Float3:	return GL_FLOAT; break;
		case Cgr::ShaderDataType::Float4:	return GL_FLOAT; break;
		case Cgr::ShaderDataType::Mat3:		return GL_FLOAT; break;
		case Cgr::ShaderDataType::Mat4:		return GL_FLOAT; break;
		case Cgr::ShaderDataType::Int:		return GL_INT; break;
		case Cgr::ShaderDataType::Int2:		return GL_INT; break;
		case Cgr::ShaderDataType::Int3:		return GL_INT; break;
		case Cgr::ShaderDataType::Int4:		return GL_INT; break;
		case Cgr::ShaderDataType::Bool:		return GL_BOOL; break;
		default: break;
		}

		CGR_CORE_ASSERT(false, "Unknown shader data type");
		return 0;
	}

	OpenGLShader::OpenGLShader(const std::string& shaderPath)
		: m_ShaderPath(shaderPath)
	{
		PrepareShader();
	}

	void OpenGLShader::PrepareShader()
	{
		const std::string& source = ParseShader(m_ShaderPath);
		const auto& shaderSources = PreProcess(source);
		CompileShaders(shaderSources);
	}

	std::string OpenGLShader::ParseShader(const std::string& shaderPath)
	{
		std::string content;
		std::ifstream in(shaderPath, std::ios::in | std::ios::binary);

		if (in)
		{
			in.seekg(0, std::ios::end);
			content.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&content[0], content.size());
			in.close();
		}
		else
		{
			CGR_CORE_ERROR("Could not open file {0}", shaderPath);
		}

		return content;
	}

	std::unordered_map<ShaderType, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		std::unordered_map<ShaderType, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);

		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			CGR_CORE_ASSERT(eol != std::string::npos, "Syntax Error");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			CGR_CORE_ASSERT((bool)ShaderTypeFromString(type), "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			shaderSources[ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}

		return shaderSources;
	}

	void OpenGLShader::CompileShaders(const std::unordered_map<ShaderType, std::string>& shaderSources)
	{
		size_t shaderCount = shaderSources.size();
		std::vector<uint32_t> shaders;
		shaders.reserve(shaderCount);

		for (auto& e : shaderSources)
		{
			ShaderType shaderType = e.first;
			const char* shaderSource = e.second.c_str();
			GLenum glType = ToOpenGLShaderType(shaderType);
			uint32_t shader = glCreateShader(glType);
			glShaderSource(shader, 1, &shaderSource, nullptr);
			glCompileShader(shader);

			int  success;
			char infoLog[512];
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

			if (!success)
			{
				glGetShaderInfoLog(shader, 512, nullptr, infoLog);
				CGR_TRACE("Vertex shader compilation status: {0}", infoLog);
			}
			else
				shaders.push_back(shader);
		}

		uint32_t program = glCreateProgram();
		for (auto e : shaders)
			glAttachShader(program, e);

		glLinkProgram(program);
		glUseProgram(program);

		m_RendererID = program;

		for (auto e : shaders)
		{
			glDetachShader(program, e);
			glDeleteShader(e);
		}
	}

	const std::string& OpenGLShader::GetPath()
	{
		return m_ShaderPath;
	}

	void OpenGLShader::Set1i(const std::string& name, int value)
	{
		UploadUniform1i(name, value);
	}

	void OpenGLShader::Set2i(const std::string& name, const glm::uvec2& value)
	{
		UploadUniform2i(name, value);
	}

	void OpenGLShader::Set3i(const std::string& name, const glm::uvec3& value)
	{
		UploadUniform3i(name, value);
	}

	void OpenGLShader::Set4i(const std::string& name, const glm::uvec4& value)
	{
		UploadUniform4i(name, value);
	}

	void OpenGLShader::Set1iArray(const std::string& name, int* value, uint32_t count)
	{
		UploadUniform1iArray(name, value, count);
	}

	void OpenGLShader::Set1f(const std::string& name, float value)
	{
		UploadUniform1f(name, value);
	}

	void OpenGLShader::Set2f(const std::string& name, const glm::vec2& value)
	{
		UploadUniform2f(name, value);
	}

	void OpenGLShader::Set3f(const std::string& name, const glm::vec3& value)
	{
		UploadUniform3f(name, value);
	}

	void OpenGLShader::Set4f(const std::string& name, const glm::vec4& value)
	{
		UploadUniform4f(name, value);
	}

	void OpenGLShader::SetMat3f(const std::string& name, const glm::mat3& matrix)
	{
		UploadUniformMat3f(name, matrix);
	}

	void OpenGLShader::SetMat4f(const std::string& name, const glm::mat4& matrix)
	{
		UploadUniformMat4f(name, matrix);
	}

	void OpenGLShader::UploadUniform1i(const std::string& name, int value)
	{
		glUniform1i(GetUniformLocation(name), value);
	}

	void OpenGLShader::UploadUniform2i(const std::string& name, const glm::uvec2& value)
	{
		glUniform2i(GetUniformLocation(name), value.x, value.y);
	}

	void OpenGLShader::UploadUniform3i(const std::string& name, const glm::uvec3& value)
	{
		glUniform3i(GetUniformLocation(name), value.x, value.y, value.z);
	}

	void OpenGLShader::UploadUniform4i(const std::string& name, const glm::uvec4& value)
	{
		glUniform4i(GetUniformLocation(name), value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::UploadUniform1iArray(const std::string& name, int* value, uint32_t count)
	{
		glUniform1iv(GetUniformLocation(name), count, value);
	}

	void OpenGLShader::UploadUniform1f(const std::string& name, float value)
	{
		glUniform1f(GetUniformLocation(name), value);
	}

	void OpenGLShader::UploadUniform2f(const std::string& name, const glm::vec2& value)
	{
		glUniform2f(GetUniformLocation(name), value.x, value.y);
	}

	void OpenGLShader::UploadUniform3f(const std::string& name, const glm::vec3& value)
	{
		glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
	}

	void OpenGLShader::UploadUniform4f(const std::string& name, const glm::vec4& value)
	{
		glUniform4f(GetUniformLocation(name), value.r, value.g, value.b, value.a);
	}

	void OpenGLShader::UploadUniformMat3f(const std::string& name, const glm::mat3& matrix)
	{
		glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4f(const std::string& name, const glm::mat4& matrix)
	{
		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::ExtractSSBOParameters(Material* material)
	{
		GLenum properties[] = { GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE, GL_NUM_ACTIVE_VARIABLES };
		GLint values[3];
		glGetProgramResourceiv(m_RendererID, GL_SHADER_STORAGE_BLOCK, 0, 3, properties, 3, nullptr, values);

		CGR_CORE_INFO("Binding: {0}, Size: {1}, Active variables: {2}", values[0], values[1], values[2]);

		for (int i = 0; i < values[2]; i++)
		{
			char varName[256];
			GLsizei varLength;
			GLenum varProps[] = { GL_TYPE, GL_OFFSET };
			GLint varValues[2];

			glGetProgramResourceName(m_RendererID, GL_BUFFER_VARIABLE, i, sizeof(varName), &varLength, varName);
			glGetProgramResourceiv(m_RendererID, GL_BUFFER_VARIABLE, i, 2, varProps, 2, nullptr, varValues);

			GLenum type = varValues[0];
			GLenum offset = varValues[1];

			CGR_CORE_TRACE("Variable name: {0}, Type: {1}, Offset: {2}", varName, type, offset);

			switch (type)
			{
			case GL_INT:
				material->AddVariable<ShaderInt>(varName, 0, offset);
				break;
			case GL_INT_VEC2:
				material->AddVariable<ShaderInt2>(varName, glm::ivec2{ 0 }, offset);
				break;
			case GL_INT_VEC3:
				material->AddVariable<ShaderInt3>(varName, glm::ivec3{ 0 }, offset);
				break;
			case GL_INT_VEC4:
				material->AddVariable<ShaderInt4>(varName, glm::ivec4{ 0 }, offset);
				break;
			case GL_FLOAT:
				material->AddVariable<ShaderFloat>(varName, 0.0f, offset);
				break;
			case GL_FLOAT_VEC2:
				material->AddVariable<ShaderFloat2>(varName, glm::vec2{ 0.0f }, offset);
				break;
			case GL_FLOAT_VEC3:
				material->AddVariable<ShaderFloat3>(varName, glm::vec3{ 0.0f }, offset);
				break;
			case GL_FLOAT_VEC4:
				material->AddVariable<ShaderFloat4>(varName, glm::vec4{ 0.0f }, offset);
				break;
			case GL_FLOAT_MAT3:
				material->AddVariable<ShaderMat3>(varName, glm::mat3{ 1.0f }, offset);
				break;
			case GL_FLOAT_MAT4:
				material->AddVariable<ShaderMat4>(varName, glm::mat4{ 1.0f }, offset);
				break;
			case GL_SAMPLER_2D:
				break;  // No default value needed for samplers
			default:
				CGR_CORE_ERROR("Unknown shader datatype variable name : {0}, GLType : {1}", varName, varValues[0]);
				break;
			}
		}
	}

	void OpenGLShader::UpdateSSBOParameters(Material* material, Ref<ShaderStorageBuffer> SSBO)
	{

		int64_t offset = 0;
		for (auto& parameter : material->GetAllVariables())
		{
			void* value = parameter->GetValue();
			int64_t offset = parameter->GetOffset();
			int64_t size = 0;
			
			switch (parameter->GetType())
			{
			case Cgr::ShaderDataType::None:
				size = 0;
				break;
			case Cgr::ShaderDataType::Float:
				size = 4;
				break;
			case Cgr::ShaderDataType::Float2:
				size = 4 * 2;
				break;
			case Cgr::ShaderDataType::Float3:
				size = 4 * 3;
				break;
			case Cgr::ShaderDataType::Float4:
				size = 4 * 4;
				break;
			case Cgr::ShaderDataType::Mat3:
				size = 4 * 3 * 3;
				break;
			case Cgr::ShaderDataType::Mat4:
				size = 4 * 4 * 4;
				break;
			case Cgr::ShaderDataType::Int:
				size = 4;
				break;
			case Cgr::ShaderDataType::Int2:
				size = 4 * 2;
				break;
			case Cgr::ShaderDataType::Int3:
				size = 4 * 3;
				break;
			case Cgr::ShaderDataType::Int4:
				size = 4 * 4;
				break;
			case Cgr::ShaderDataType::Bool:
				size = 1;
				break;
			default:
				break;
			}

			CGR_CORE_ASSERT(size, "Invalid shader parameter");
			SSBO->SetData(offset, size, value);

			//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

			// Memory barrier to ensure GPU sees the updated data
			//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}
	}

	uint32_t OpenGLShader::GetUniformLocation(const std::string& name)
	{
		if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
			return m_UniformLocationCache[name];

		int location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
			CGR_CORE_WARN("Warning uniform {0} doesn't exist!", name);
		m_UniformLocationCache[name] = location;
		return location;
	}

	void OpenGLShader::Bind()
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind()
	{
		glUseProgram(0);
	}
}