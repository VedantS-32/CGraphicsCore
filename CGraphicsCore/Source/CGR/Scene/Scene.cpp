#include "CGRpch.h"
#include "Scene.h"

#include "CGR/Core/Application.h"
#include "CGR/Renderer/Renderer.h"
#include "Entity.h"
#include "Components.h"
#include "CGR/Core/Timestep.h"
#include "CGR/Renderer/Camera.h"
#include <glm/gtc/type_ptr.hpp>

namespace Cgr
{
	Scene::Scene()
		: m_Name("Untitled")
	{
		m_Renderer = Application::Get().GetRenderer();
	}

	Scene::Scene(const std::string& name)
		: m_Name(name)
	{
		m_Renderer = Application::Get().GetRenderer();
	}

    Entity Scene::CreateEntity(const std::string& name)
    {
        Entity entity = { m_Registry.create(), this};
        auto& tag = entity.AddComponent<TagComponent>();
		entity.AddComponent<TransformComponent>();
        tag.Tag = name.empty() ? "Entity" : name;
        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_Registry.destroy(entity);
    }

    void Scene::OnUpdate(Timestep ts, Camera& camera)
    {
		auto models = m_Registry.view<ModelComponent, TransformComponent>();
		for (auto entity : models)
		{
			auto [model, transform] = models.get<ModelComponent, TransformComponent>(entity);
			auto modelProps = m_Renderer->GetModelPropsUniformBuffer();
			int entityID = static_cast<int>(entity);
			modelProps->SetData(0, sizeof(uint32_t) * 4, &entityID);
			modelProps->SetData(sizeof(uint32_t) * 4, sizeof(glm::mat4), glm::value_ptr(model.GetModel()->GetModelMatrix()));
			model.GetModel()->UpdateTransform(transform.GetTransform());
			m_Renderer->DrawModel(model.GetModel());
		}
    }

	void Scene::OnViewportResize(float width, float height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
	}

	template<>
	CGR_API void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
		CGR_CORE_TRACE("Added Transform Component");
	}

	template<>
	CGR_API void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
		CGR_CORE_TRACE("Added Tag Component");
	}

	template<>
	CGR_API void Scene::OnComponentAdded<ModelComponent>(Entity entity, ModelComponent& component)
	{
		CGR_CORE_TRACE("Added Model Component");
		auto assetManager = Application::Get().GetAssetManager();
		component.SetModel(assetManager->GetAsset<Model>(assetManager->GetDefaultAssetHandle(AssetType::Model)));
		auto model = component.GetModel();
		int currentMatIdx = -1;
		for (auto& mesh : model->GetMeshes())
		{
			if (currentMatIdx != mesh.GetMaterialIndex())
			{
				currentMatIdx = mesh.GetMaterialIndex();
				m_Renderer->m_WorldSettings->SetBlockBinding(model->GetMaterial(currentMatIdx)->GetShader()->GetRendererID());
				m_Renderer->GetModelCommonsUniformBuffer()->SetBlockBinding(model->GetMaterial(currentMatIdx)->GetShader()->GetRendererID());
				m_Renderer->GetModelPropsUniformBuffer()->SetBlockBinding(model->GetMaterial(currentMatIdx)->GetShader()->GetRendererID());
			}
		}
	}
}
