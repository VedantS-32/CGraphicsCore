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

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		//Always call Renderer::OnUpdate before calling ActiveScene::OnUpdate, as it updates shader buffer
		void OnUpdate(Timestep ts, Camera& camera);
		void OnViewportResize(float width, float height);
		const EntityRegistry& GetRegistry() { return m_Registry; }

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

	private:
		float m_ViewportWidth = 1, m_ViewportHeight = 1;

		Renderer* m_Renderer;
		EntityRegistry m_Registry;
	};
}