#include "CGRpch.h"
#include "Camera.h"

#include "CGR/Core/Input.h"
#include "CGR/Core/KeyCode.h"
#include "CGR/Core/MouseButtonCode.h"
#include "CGR/Core/InputMode.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace Cgr
{
	void Camera::OnUpdate(Timestep ts)
	{
		const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
		glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.075f;
		m_InitialMousePosition = mouse;

		if (Input::IsMouseButtonPressed(Mouse::CGR_BUTTON_RIGHT))
		{
			Input::SetInputMode(InputMode::CGR_CURSOR, CursorMode::CGR_CURSOR_DISABLED);
			m_MouseHidden = true;
			m_IsMoving = true;

			UpdateOrientation(delta);

			if (Input::IsKeyPressed(Key::CGR_KEY_W))
				m_Position += m_Speed * m_Orientation;
			if (Input::IsKeyPressed(Key::CGR_KEY_S))
				m_Position -= m_Speed * m_Orientation;
			if (Input::IsKeyPressed(Key::CGR_KEY_A))
				m_Position += m_Speed * glm::cross(m_UpDirection, m_Orientation);
			if (Input::IsKeyPressed(Key::CGR_KEY_D))
				m_Position -= m_Speed * glm::cross(m_UpDirection, m_Orientation);
			if (Input::IsKeyPressed(Key::CGR_KEY_Q))
				m_Position -= m_Speed * m_UpDirection;
			if (Input::IsKeyPressed(Key::CGR_KEY_E))
				m_Position += m_Speed * m_UpDirection;
		}
		else if (m_MouseHidden)
		{
			Input::SetInputMode(InputMode::CGR_CURSOR, CursorMode::CGR_CURSOR_NORMAL);
			m_MouseHidden = false;
			m_IsMoving = false;
		}

		UpdateViewMatrix();
	}

	void Camera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(CGR_BIND_EVENT_FN(Camera::OnMouseScroll));
	}

	void Camera::SetPerspective(double fov, double nearClip, double farClip)
	{
		m_ProjectionType = ProjectionType::Perspective;
		m_PerspectiveFov = fov;
		m_PerspectiveNear = nearClip;
		m_PerspectiveFar = farClip;
		
		UpdateProjectionMatrix();
	}

	void Camera::SetOrthographic(double size, double nearClip, double farClip)
	{
		m_ProjectionType = ProjectionType::Orthographic;
		m_OrthographicSize = size;
		m_OrthographicNear = nearClip;
		m_OrthographicFar = farClip;

		UpdateProjectionMatrix();
	}

	void Camera::SetViewportAspectRatio(float width, float height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		UpdateProjectionMatrix();
		UpdateViewMatrix();
	}

	bool Camera::OnMouseScroll(MouseScrolledEvent& e)
	{
		if (Input::IsMouseButtonPressed(Mouse::CGR_BUTTON_RIGHT))
		{
			float delta = e.YOffset * 0.005f;
			m_Speed += delta;
			m_Speed = std::clamp(m_Speed, 0.001f, 100.0f);
		}

		return false;

	}

	void Camera::UpdateOrientation(const glm::vec2& delta)
	{
		auto newOrientation = glm::rotate(m_Orientation, glm::radians(delta.y), glm::normalize(glm::cross(m_UpDirection, m_Orientation)));

		if (!((glm::angle(newOrientation, m_UpDirection) <= glm::radians(0.1f)) or (glm::angle(newOrientation, -m_UpDirection) <= glm::radians(0.1f))))
			m_Orientation = newOrientation;

		m_Orientation = glm::rotate(m_Orientation, glm::radians(-delta.x), m_UpDirection);

	}

	void Camera::UpdateProjectionMatrix()
	{
		double aspectRatio = m_ViewportWidth / m_ViewportHeight;

		switch (m_ProjectionType)
		{
		case ProjectionType::Perspective:
		{
			m_ProjectionMatrix = glm::perspective<double>(glm::radians(m_PerspectiveFov), aspectRatio, m_PerspectiveNear, m_PerspectiveFar);
			break;
		}
		case ProjectionType::Orthographic:
		{
			double orthoLeft = -0.5f * aspectRatio * m_OrthographicSize;
			double orthoRight = 0.5f * aspectRatio * m_OrthographicSize;
			double orthoBottom = -0.5f * m_OrthographicSize;
			double orthoTop = 0.5f * m_OrthographicSize;

			m_ProjectionMatrix = glm::ortho<double>(orthoLeft, orthoRight, orthoBottom,
				orthoTop, m_OrthographicNear, m_OrthographicFar);
			break;
		}
		default:
			break;
		}
	}

	void Camera::UpdateViewMatrix()
	{
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Orientation, m_UpDirection);
		UpdateViewProjectionMatrix();
	}

	void Camera::UpdateViewProjectionMatrix()
	{
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
}
