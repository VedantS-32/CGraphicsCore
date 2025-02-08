#include "CGRpch.h"
#include "Entity.h"

namespace Cgr
{
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_Handle(handle), m_Scene(scene)
	{
	}
}
