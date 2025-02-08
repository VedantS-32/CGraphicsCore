#pragma once

#include "CGR/Core/Core.h"

#include <entt.hpp>

namespace Cgr
{
	// Alias for entt::registry
	using EntityRegistry = entt::registry;

	class Entity;
	class Timestep;
	class Camera;
	class Renderer;

	class CGR_API Scene
	{
	public:
		friend class Entity;

		Scene();
		Scene(const std::string& name);

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);
		
		const std::string& GetName() { return m_Name; }
		void SetName(const std::string& name) { m_Name = name; }

		//Always call Renderer::OnUpdate before calling ActiveScene::OnUpdate, as it updates shader buffer
		void OnUpdate(Timestep ts, Camera& camera);
		void OnViewportResize(float width, float height);
		const EntityRegistry& GetRegistry() { return m_Registry; }

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

	private:
		float m_ViewportWidth = 1, m_ViewportHeight = 1;

		std::string m_Name;
		std::filesystem::path m_Path;
		Renderer* m_Renderer;
		EntityRegistry m_Registry;
	};
}