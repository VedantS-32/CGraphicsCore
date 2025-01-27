#include "CGRpch.h"
#include "MaterialSerializer.h"

#include "CGR/Core/Application.h"
#include "CGR/Asset/AssetManager.h"
#include "CGR/Utils/YamlOperators.h"

namespace Cgr
{
    MaterialSerializer::MaterialSerializer(Ref<Material> material)
        : m_Material(material)
    {
    }

    void MaterialSerializer::Serialize(const std::filesystem::path& path)
    {
        const auto assetManager = Application::Get().GetAssetManager();
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Asset" << YAML::Value << m_Material->Handle;
        out << YAML::Key << "Material" << YAML::Value << m_Material->Name;
        out << YAML::Key << "Shader" << YAML::Value << assetManager->GetFilePath(m_Material->GetShader()->Handle).string();

        out << YAML::Key << "Textures" << YAML::Value << YAML::BeginMap;
        const auto& textures = m_Material->GetAllTextures();
        for (auto& [uuid, texture] : textures)
        {
            out << YAML::Key << texture->GetName() << YAML::Value << assetManager->GetFilePath(texture->Handle).string();
        }
        out << YAML::EndMap;

        out << YAML::Key << "Attributes" << YAML::Value << YAML::BeginMap;
        const auto& materialParameters = m_Material->GetAllVariables();
        for (auto& param : materialParameters)
        {
            switch (param->GetType())
            {
            case Cgr::ShaderDataType::None:
                break;
            case Cgr::ShaderDataType::Float:
                out << YAML::Key << param->GetName() << YAML::Value << *static_cast<float*>(param->GetValue());
                break;
            case Cgr::ShaderDataType::Float2:
                out << YAML::Key << param->GetName() << YAML::Value << *static_cast<glm::vec2*>(param->GetValue());
                break;
            case Cgr::ShaderDataType::Float3:
                out << YAML::Key << param->GetName() << YAML::Value << *static_cast<glm::vec3*>(param->GetValue());
                break;
            case Cgr::ShaderDataType::Float4:
                out << YAML::Key << param->GetName() << YAML::Value << *static_cast<glm::vec4*>(param->GetValue());
                break;
            case Cgr::ShaderDataType::Mat3:
                //out << YAML::Key << param->GetName() << YAML::Value << static_cast<float*>(param->GetValue()));
                break;
            case Cgr::ShaderDataType::Mat4:
                //out << YAML::Key << param->GetName() << YAML::Value << static_cast<float*>(param->GetValue()));
                break;
            case Cgr::ShaderDataType::Int:
                out << YAML::Key << param->GetName() << YAML::Value << *static_cast<int*>(param->GetValue());
                break;
            case Cgr::ShaderDataType::Int2:
                out << YAML::Key << param->GetName() << YAML::Value << *static_cast<glm::ivec2*>(param->GetValue());
                break;
            case Cgr::ShaderDataType::Int3:
                out << YAML::Key << param->GetName() << YAML::Value << *static_cast<glm::ivec3*>(param->GetValue());
                break;
            case Cgr::ShaderDataType::Int4:
                out << YAML::Key << param->GetName() << YAML::Value << *static_cast<glm::ivec4*>(param->GetValue());
                break;
            case Cgr::ShaderDataType::Bool:
                out << YAML::Key << param->GetName() << YAML::Value << *static_cast<bool*>(param->GetValue());
                break;
            default:
                break;
            }
        }
        out << YAML::EndMap;
        out << YAML::EndMap;

        if(!std::filesystem::exists(path))
			std::filesystem::create_directories(path.parent_path());

        std::ofstream fout(path);
        fout << out.c_str();

        CGR_CORE_INFO("Serialized material to: {0}", path.string());
    }

    void MaterialSerializer::Deserialize(const std::filesystem::path& path)
    {
        std::ifstream stream(path);
        std::stringstream strStream;
        strStream << stream.rdbuf();

        YAML::Node data = YAML::Load(strStream.str());
        if (!data["Asset"])
            return;

        const auto assetManager = Application::Get().GetAssetManager();
        m_Material->Name = data["Material"].as<std::string>();

        auto shader = assetManager->GetAsset<Shader>(assetManager->ImportAsset(data["Shader"].as<std::string>()));
        m_Material->SetShader(shader);
        m_Material->GetAllTextures().clear();
        auto textures = data["Textures"];
        for (auto texture : textures)
        {
            UUID uuid;
            auto importedTex = assetManager->GetAsset<Texture>(assetManager->ImportAsset(texture.second.as<std::string>()));
            m_Material->AddTexture(uuid, importedTex);
        }

        auto& parameters = m_Material->GetAllVariables();
        auto attribute = data["Attributes"];
        for (auto& parameter : parameters)
        {
            switch (parameter->GetType())
            {
            case ShaderDataType::None:
                break;
            case ShaderDataType::Float:
                *static_cast<float*>(parameter->GetValue()) = attribute[parameter->GetName()].as<float>();
                break;
            case ShaderDataType::Float2:
                *static_cast<glm::vec2*>(parameter->GetValue()) = attribute[parameter->GetName()].as<glm::vec2>();
                break;
            case ShaderDataType::Float3:
                *static_cast<glm::vec3*>(parameter->GetValue()) = attribute[parameter->GetName()].as<glm::vec3>();
                break;
            case ShaderDataType::Float4:
                *static_cast<glm::vec4*>(parameter->GetValue()) = attribute[parameter->GetName()].as<glm::vec4>();
                break;
            case ShaderDataType::Mat3:
                //out << YAML::Key << param->GetName() << YAML::Value << static_cast<float*>(param->GetValue()));
                break;
            case ShaderDataType::Mat4:
                //out << YAML::Key << param->GetName() << YAML::Value << static_cast<float*>(param->GetValue()));
                break;
            case ShaderDataType::Int:
                *static_cast<int*>(parameter->GetValue()) = attribute[parameter->GetName()].as<int>();
                break;
            case ShaderDataType::Int2:
                *static_cast<glm::ivec2*>(parameter->GetValue()) = attribute[parameter->GetName()].as<glm::ivec2>();
                break;
            case ShaderDataType::Int3:
                *static_cast<glm::ivec3*>(parameter->GetValue()) = attribute[parameter->GetName()].as<glm::ivec3>();
                break;
            case ShaderDataType::Int4:
                *static_cast<glm::ivec4*>(parameter->GetValue()) = attribute[parameter->GetName()].as<glm::ivec4>();
                break;
            case ShaderDataType::Bool:
                *static_cast<bool*>(parameter->GetValue()) = attribute[parameter->GetName()].as<bool>();
                break;
            default:
                break;
            }
        }

        CGR_CORE_INFO("Deserialized material from: {0}", path.string());
    }
}
