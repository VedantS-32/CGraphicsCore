#include "CGRpch.h"
#include "SceneSerializer.h"

#include "CGR/Scene/Entity.h"
#include "CGR/Scene/Components.h"
#include "CGR/Core/Application.h"
#include "CGR/Asset/AssetManager.h"
#include "CGR/Utils/YamlOperators.h"

namespace Cgr
{
    static void SerializeEntity(YAML::Emitter& out, Entity entity)
    {
        const auto assetManager = Application::Get().GetAssetManager();
		out << YAML::BeginMap; //Entity
		out << YAML::Key << "Entity" << YAML::Value << static_cast<uint32_t>(entity.GetHandle()); // TODO: Entity ID goes here

		out << YAML::Key << "TagComponent";
		out << YAML::BeginMap; // TagComponent

		auto& tag = entity.GetComponent<TagComponent>().Tag;
		out << YAML::Key << "Tag" << YAML::Value << tag;

		out << YAML::EndMap; // TagComponent

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent

			auto& transform = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << transform.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << transform.Scale;

			out << YAML::EndMap; // TransformComponent
		}

		if (entity.HasComponent<ModelComponent>())
		{
			out << YAML::Key << "ModelComponent";
			out << YAML::BeginMap; // ModelComponent

			auto& modelComponent = entity.GetComponent<ModelComponent>();
            auto handle = modelComponent.GetModel()->Handle;
            const auto& path = assetManager->GetFilePath(handle);
            out << YAML::Key << "Model" << path.string();

			// Material Serialization is Handled by MaterialSerializer Class

			out << YAML::EndMap; // ModelComponent
		}

		out << YAML::EndMap; // Entity;

		CGR_CORE_TRACE("Serialized Entity: {0}", tag);

    }

    SceneSerializer::SceneSerializer(Ref<Scene> Scene)
        : m_Scene(Scene)
    {
    }

    void SceneSerializer::Serialize(const std::filesystem::path& path)
    {
        const auto& filePath = path;
        const auto assetManager = Application::Get().GetAssetManager();
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << m_Scene->GetName();
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        m_Scene->GetRegistry().view<entt::entity>().each([&](auto entityID)
            {
                Entity entity = { entityID, m_Scene.get() };
                if (!entity)
                    return;

                SerializeEntity(out, entity);
            });
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(path);
        fout << out.c_str();

        CGR_CORE_INFO("Serialized Scene to: {0}", path.string());
    }

    void SceneSerializer::Deserialize(const std::filesystem::path& path)
    {
        std::ifstream stream(path);
        std::stringstream strStream;
        strStream << stream.rdbuf();

        YAML::Node data = YAML::Load(strStream.str());
        if (!data["Scene"])
            return;

        const auto assetManager = Application::Get().GetAssetManager();
        m_Scene->SetName(data["Scene"].as<std::string>());

        //m_Scene->SetPath(data["Source"].as<std::string>());
		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				Entity deserializedEntity = m_Scene->CreateEntity(name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					// An Entity will always have transforms
					auto& transform = deserializedEntity.GetComponent<TransformComponent>();
					transform.Translation = transformComponent["Translation"].as<glm::vec3>();
					transform.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					transform.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				auto modelComponent = entity["ModelComponent"];
				if (modelComponent)
				{
					auto handle = assetManager->ImportAsset(modelComponent["Model"].as<std::string>());
					auto& model = deserializedEntity.AddComponent<ModelComponent>();
					model.SetModel(assetManager->GetAsset<Model>(handle));
				}
			}
		}

        CGR_CORE_INFO("Deserialized Scene from: {0}", path.string());
    }
}
