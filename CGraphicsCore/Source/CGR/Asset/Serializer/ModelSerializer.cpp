#include "CGRpch.h"
#include "ModelSerializer.h"

#include "MaterialSerializer.h"

#include "CGR/Core/Application.h"
#include "CGR/Asset/AssetManager.h"
#include "CGR/Utils/YamlOperators.h"

namespace Cgr
{
    ModelSerializer::ModelSerializer(Ref<Model> Model)
        : m_Model(Model)
    {
    }

    void ModelSerializer::Serialize(const std::filesystem::path& path)
    {
        auto filePath = path;
        const auto assetManager = Application::Get().GetAssetManager();
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Asset" << YAML::Value << m_Model->Handle;
        out << YAML::Key << "Model" << YAML::Value << m_Model->Name;
        out << YAML::Key << "Source" << YAML::Value << m_Model->GetPath();

        out << YAML::Key << "Materials" << YAML::Value << YAML::BeginMap;
        const auto& materials = m_Model->GetAllMaterials();
        uint32_t i = 0;
        for (auto& material : materials)
        {
            auto& materialPath = assetManager->GetFilePath(material->Handle);
            MaterialSerializer serializer(material);
            serializer.Serialize(materialPath);
            out << YAML::Key << i << YAML::Value << materialPath.string();
            i++;
        }
        out << YAML::EndMap;
        out << YAML::EndMap;

        if (!(path.extension() == ".csmesh"))
        {
            auto assetManager = Application::Get().GetAssetManager();
            filePath = std::format("Content/Generated/Model/{}.csmesh", path.stem().string());
        }

        if (!std::filesystem::exists(filePath))
            std::filesystem::create_directories(filePath.parent_path());

        std::ofstream fout(filePath);
        fout << out.c_str();

        CGR_CORE_INFO("Serialized model to: {0}", path.string());
    }

    void ModelSerializer::Deserialize(const std::filesystem::path& path)
    {
        std::ifstream stream(path);
        std::stringstream strStream;
        strStream << stream.rdbuf();

        YAML::Node data = YAML::Load(strStream.str());
        if (!data["Asset"])
            return;

        const auto assetManager = Application::Get().GetAssetManager();
        m_Model->Name = data["Model"].as<std::string>();

        m_Model->SetPath(data["Source"].as<std::string>());
        m_Model->GetAllMaterials().clear();

        auto materialPaths = data["Materials"];
        for (auto path : materialPaths)
        {
            auto material = assetManager->GetAsset<Material>(assetManager->ImportAsset(path.second.as<std::string>()));
            MaterialSerializer serializer(material);
            serializer.Deserialize(path.second.as<std::string>());
            m_Model->AddMaterial(material);
        }

        CGR_CORE_INFO("Deserialized model from: {0}", path.string());
    }
}
