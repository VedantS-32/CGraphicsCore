#include "CGRpch.h"
#include "Shader.h"

#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Cgr
{
    Ref<Shader> Shader::Create(const std::string& name, const std::string& shaderPath)
    {
        switch (RendererAPI::GetAPI())
        {
        case Cgr::API::None:
            CGR_CORE_ASSERT("No Graphics API selected");
            break;
        case Cgr::API::OpenGL:
            return CreateRef<OpenGLShader>(name, shaderPath);
            break;
        default:
            break;
        }

        return nullptr;
    }

    ShaderLibrary::ShaderLibrary()
    {
        Add("Content/Shader/Cube.glsl");
    }

    void ShaderLibrary::Add(const std::string& shaderPath)
    {
        // Extract name from filepath
        auto lastSlash = shaderPath.find_last_of("/\\");
        lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
        auto lastDot = shaderPath.rfind('.');
        auto count = lastDot == std::string::npos ? shaderPath.size() - lastSlash : lastDot - lastSlash;
        auto name = shaderPath.substr(lastSlash, count);

        if (!Exists(name))
        {
            auto shader = Shader::Create(name, shaderPath);
            Add(name, shader);
        }
    }

    void ShaderLibrary::Add(Ref<Shader> shader)
    {
        auto& shaderPath = shader->GetPath();

        auto lastSlash = shaderPath.find_last_of("/\\");
        lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
        auto lastDot = shaderPath.rfind('.');
        auto count = lastDot == std::string::npos ? shaderPath.size() - lastSlash : lastDot - lastSlash;
        auto name = shaderPath.substr(lastSlash, count);

        if (!Exists(name))
            Add(name, shader);
    }

    void ShaderLibrary::Add(const std::string& name, Ref<Shader> shader)
    {
        m_Shaders[name] = shader;
    }

    bool ShaderLibrary::Exists(const std::string& name)
    {
        return m_Shaders.find(name) != m_Shaders.end();
    }

    Ref<Shader> ShaderLibrary::Get(const std::string& name)
    {
        CGR_CORE_ASSERT(Exists(name), "Shader not found!");
        return m_Shaders[name];
    }

    Ref<ShaderLibrary> ShaderLibrary::Create()
    {
        return CreateRef<ShaderLibrary>();
    }
}