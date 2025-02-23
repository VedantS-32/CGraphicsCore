#pragma once

#include "CGR/Core/Core.h"
#include "CGR/Core/Reflection.h"
#include "CGR/Core/Timestep.h"
#include "CGR/Event/Event.h"
#include "CGR/Event/MouseEvent.h"

#define GLM_ENABLE_EXPERIMENTAL 
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Cgr
{
	enum class CGR_API ProjectionType
	{
		Perspective,
		Orthographic
	};

	class CGR_API Camera
	{
	public:
		CLASS(Camera);
		Camera();
		virtual ~Camera() {}

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		void SetPerspective(float fov, double nearClip = 0.01f, double farClip = 10000.0f);
		void SetOrthographic(double size, double nearClip = -1000.0f, double farClip = 1000.0f);
		void SetViewportSize(float width, float height);

		const glm::mat4& GetProjectionMatrix() { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() { return m_ViewProjectionMatrix; }
		void UpdateProjectionMatrix();

	public:
		const glm::vec3& GetPosition() { return m_Position; }
		bool IsMouseMoving() const { return m_IsMoving; }

	private:
		bool OnMouseScroll(MouseScrolledEvent& e);

	private:
		void UpdateOrientation(const glm::vec2& delta);

		void UpdateViewMatrix();
		void UpdateViewProjectionMatrix();

	private:
		glm::vec2 m_InitialMousePosition{ 0.0f };
		float m_Speed = 0.025f;
		bool m_MouseHidden = false;
		bool m_IsMoving = false;

	private:
		ProjectionType m_ProjectionType = ProjectionType::Perspective;

		glm::mat4 m_ProjectionMatrix{ 1.0f };
		glm::mat4 m_ViewMatrix{ 1.0f };
		glm::mat4 m_ViewProjectionMatrix{ 1.0f };

		glm::vec3 m_Position{ 0.0f, -10.0f, 5.0f };
		glm::vec3 m_Orientation{ 0.0f, 1.0f, -0.5f };
		glm::vec3 m_UpDirection{ 0.0f, 0.0f, 1.0f };

		float m_PerspectiveFov = 45.0f;
		double m_PerspectiveNear = 0.1f, m_PerspectiveFar = 10000.0f;

		double m_OrthographicSize = 10.0f;
		double m_OrthographicNear = -1000.0f, m_OrthographicFar = 1000.0f;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;
	};
}