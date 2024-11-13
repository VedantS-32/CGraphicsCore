#pragma once
#include <string>

namespace Cgr
{
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
							   virtual EventType GetEventType() const override { return GetStaticType(); }\
							   virtual const char* GetName() const override { return #type; }

	enum class EventType
	{
		None = 0,
		WindowResized, WindowMoved, WindowFocused, WindowLostFocus, WindowClosed,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseButtonClicked, MouseMoved, MouseScrolled
	};

	class Event
	{
		friend class EventHandler;

	public:
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual std::string ToString() const = 0;

	private:
		bool Handled = false;
	};

	class EventHandler
	{
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}