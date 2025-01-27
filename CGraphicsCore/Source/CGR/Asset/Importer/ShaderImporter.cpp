#include "CGRpch.h"
#include "ShaderImporter.h"

namespace Cgr
{
    namespace Utils
    {
        static ShaderType ShaderTypeFromString(const std::string& type)
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
    }

    Ref<Shader> ShaderImporter::ImportShader(AssetHandle handle, const AssetMetadata& metadata)
    {
        return LoadShader(metadata.Path);
    }

    std::string ShaderImporter::ParseFile(const std::string& filePath)
    {
        std::string content;
        std::ifstream in(filePath, std::ios::in | std::ios::binary);

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
            CGR_CORE_ERROR("Could not open file {0}", filePath);
        }

        return content;
    }

    std::unordered_map<ShaderType, std::string> ShaderImporter::PreProcess(const std::string& source)
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
            CGR_CORE_ASSERT((bool)Utils::ShaderTypeFromString(type), "Invalid shader type specified");

            size_t nextLinePos = source.find_first_not_of("\r\n", eol);
            pos = source.find(typeToken, nextLinePos);
            shaderSources[Utils::ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
        }

        return shaderSources;
    }

    Ref<Shader> ShaderImporter::LoadShader(const std::filesystem::path& filePath)
    {
        auto source = ParseFile(filePath.string());
        auto shaderSources = PreProcess(source);
        auto shader = Shader::Create(shaderSources, filePath);
        CGR_CORE_TRACE("Imported Shader asset, path: {0}", filePath.string());

        return shader;
    }
}
