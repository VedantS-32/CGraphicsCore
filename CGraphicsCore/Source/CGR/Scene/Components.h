#pragma once

#include "CGR/Core/Core.h"
#include "CGR/Core/Application.h"
#include "CGR/Renderer/Model.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Cgr
{
	struct CGR_API TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}
	};

	struct CGR_API TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3 translation)
			: Translation(translation) {}

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4(1.0f), Translation)
				* rotation
				* glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct CGR_API ModelComponent
	{
	public:
		ModelComponent() = default;

		Ref<Model> GetModel() { return m_Model; }

		void SetModel(Ref<Model> model)
		{
			auto renderer = Application::Get().GetRenderer();
			int currentMatIdx = -1;
			for (auto& mesh : model->GetMeshes())
			{
				if (currentMatIdx != mesh.GetMaterialIndex())
				{
					currentMatIdx = mesh.GetMaterialIndex();
					renderer->m_WorldSettings->SetBlockBinding(model->GetMaterial(currentMatIdx)->GetShader()->GetRendererID());
					renderer->GetModelCommonsUniformBuffer()->SetBlockBinding(model->GetMaterial(currentMatIdx)->GetShader()->GetRendererID());
					renderer->GetModelPropsUniformBuffer()->SetBlockBinding(model->GetMaterial(currentMatIdx)->GetShader()->GetRendererID());
				}
			}

			m_Model = model;
		}

		// Always call this after changing model
		void RefreshShaderBuffer()
		{
			auto renderer = Application::Get().GetRenderer();
			int currentMatIdx = -1;
			for (auto& mesh : m_Model->GetMeshes())
			{
				if (currentMatIdx != mesh.GetMaterialIndex())
				{
					currentMatIdx = mesh.GetMaterialIndex();
					renderer->m_WorldSettings->SetBlockBinding(m_Model->GetMaterial(currentMatIdx)->GetShader()->GetRendererID());
					renderer->GetModelCommonsUniformBuffer()->SetBlockBinding(m_Model->GetMaterial(currentMatIdx)->GetShader()->GetRendererID());
					renderer->GetModelPropsUniformBuffer()->SetBlockBinding(m_Model->GetMaterial(currentMatIdx)->GetShader()->GetRendererID());
				}
			}
		}

	private:
		Ref<Model> m_Model;
	};
}