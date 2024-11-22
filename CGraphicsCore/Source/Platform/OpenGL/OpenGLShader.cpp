#include "CGRpch.h"
#include "OpenGLShader.h"

#include <glad/glad.h>

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

	void OpenGLShader::Bind()
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind()
	{
		glUseProgram(0);
	}
}