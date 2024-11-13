#pragma once

#include "Event.h"
#include "CGR/Core/MouseButtonCode.h"
#include "CGRpch.h"

namespace Cgr
{
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float xPos, float yPos)
			: XPos(xPos), YPos(yPos) {}

		virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Mouse moved event: " << XPos << ", " << YPos;

			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved);

	private:
		float XPos, YPos;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset)
			: XOffset(xOffset), YOffset(yOffset) {}

		virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Mouse scrolled event: " << XOffset << ", " << YOffset;

			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled);

	private:
		float XOffset, YOffset;
	};

	class MouseButtonEvent : public Event
	{
	public:
		MouseButtonEvent(MouseCode button)
			: m_Button(button) {}

		MouseCode GetMouseButton() const { return m_Button; }

	protected:
		MouseCode m_Button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(MouseCode button)
			: MouseButtonEvent(button) {}

		virtual std::string ToString() const
		{
			std::stringstream ss;
			ss << "Mouse button pressed event: " << m_Button;

			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed);
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(MouseCode button)
			: MouseButtonEvent(button) {}

		virtual std::string ToString() const
		{
			std::stringstream ss;
			ss << "Mouse button released event: " << m_Button;

			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased);
	};

	class MouseButtonClickedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonClickedEvent(MouseCode button)
			: MouseButtonEvent(button) {}

		virtual std::string ToString() const
		{
			std::stringstream ss;
			ss << "Mouse button clicked event: " << m_Button;

			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonClicked);
	};
}
