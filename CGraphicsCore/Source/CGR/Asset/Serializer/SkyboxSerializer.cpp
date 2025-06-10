#include "CGRpch.h"
#include "SkyboxSerializer.h"

#include "CGR/Core/Application.h"
#include "CGR/Asset/AssetManager.h"
#include "CGR/Utils/YamlOperators.h"
#include "CGR/Renderer/Renderer.h"
#include <stb_image.h>

namespace Cgr
{
    SkyboxSerializer::SkyboxSerializer(Ref<Skybox> Skybox)
        : m_Skybox(Skybox)
    {
    }

    void SkyboxSerializer::Serialize(const std::filesystem::path& path)
    {
        const auto assetManager = Application::Get().GetAssetManager();
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Asset" << YAML::Value << m_Skybox->Handle;
        out << YAML::Key << "Skybox" << YAML::Value << m_Skybox->Name;
        out << YAML::Key << "Shader" << YAML::Value << assetManager->GetFilePath(m_Skybox->GetShader()->Handle).string();

        out << YAML::Key << "Textures" << YAML::Value << YAML::BeginMap;
        const auto& texHandles = m_Skybox->GetTextureHandles();
        int i = 0;
        for (auto& [side, texHandle] : texHandles)
        {
            out << YAML::Key << Utils::SkyboxSideEnumToString(side) << YAML::Value << assetManager->GetFilePath(texHandle).string();
        }
        out << YAML::EndMap;

        out << YAML::Key << "Attributes" << YAML::Value << YAML::BeginMap;
        const auto& SkyboxParameters = m_Skybox->GetAllVariables();
        for (auto& param : SkyboxParameters)
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

        CGR_CORE_INFO("Serialized Skybox to: {0}", path.string());
    }

    void SkyboxSerializer::Deserialize(const std::filesystem::path& path)
    {
        std::ifstream stream(path);
        std::stringstream strStream;
        strStream << stream.rdbuf();

        YAML::Node data = YAML::Load(strStream.str());
        if (!data["Asset"])
            return;

        const auto assetManager = Application::Get().GetAssetManager();
        m_Skybox->Name = data["Skybox"].as<std::string>();

        auto shader = assetManager->GetAsset<Shader>(assetManager->ImportAsset(data["Shader"].as<std::string>()));
        m_Skybox->SetShader(shader);
        auto textures = data["Textures"];
        int i = 0;
        m_Skybox->GetTextureHandles().clear();
        for (auto texture : textures)
        {
            auto texHandle = assetManager->ImportAsset(texture.second.as<std::string>());
            m_Skybox->AddTextureHandle(static_cast<SkyboxSide>(i++), texHandle);
        }

        CGR_CORE_ASSERT(i == 6, "Skybox is incomplete!");

        m_Skybox->UploadTextures();

        auto& parameters = m_Skybox->GetAllVariables();
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

        CGR_CORE_INFO("Deserialized Skybox from: {0}", path.string());
    }
}
