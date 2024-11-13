#pragma once

#include "Event.h"
#include "CGRpch.h"

namespace Cgr
{
	class WindowResizedEvent : public Event
	{
	public:
		WindowResizedEvent(uint32_t width, uint32_t height)
			: Width(width), Height(height) {}

		virtual std::string ToString() const
		{
			std::stringstream ss;
			ss << "Window resized event: " << Width << "," << Height;

			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResized);

	private:
		uint32_t Width, Height;
	};

	class WindowMovedEvent : public Event
	{
	public:
		WindowMovedEvent(int xPos, int yPos)
			: XPos(xPos), YPos(yPos) {}

		virtual std::string ToString() const
		{
			std::stringstream ss;
			ss << "Window moved event: " << XPos << "," << YPos;

			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowMoved);

	private:
		int XPos, YPos;
	};

	class WindowClosedEvent : public Event
	{
	public:
		WindowClosedEvent() {}

		virtual std::string ToString() const
		{
			return "Window closed";
		}

		EVENT_CLASS_TYPE(WindowClosed);
	};
}

