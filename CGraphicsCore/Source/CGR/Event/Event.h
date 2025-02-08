#pragma once

#include "CGR/Core/Core.h"
#include "CGR/Core/Layer.h"
#include "CGR/Core/LayerStack.h"

#include <string>

namespace Cgr
{
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
							   virtual EventType GetEventType() const override { return GetStaticType(); }\
							   virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	using EventCallbackHandle = std::function<void(Event&)>;

	template<typename T>
	using EventCallbackFn = std::function<void(T&)>;

	enum class EventType
	{
		None = 0,
		WindowResized, WindowMoved, WindowFocused, WindowLostFocus, WindowClosed,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseButtonClicked, MouseMoved, MouseScrolled
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput = BIT(1),
		EventCategoryKeyboard = BIT(2),
		EventCategoryMouse = BIT(3),
		EventCategoryMouseButton = BIT(4)
	};

	class CGR_API Event
	{
		friend class EventManager;
		friend class EventDispatcher;
		friend class EventListenerInterface;
		friend class Application;

	public:
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const = 0;

		inline bool IsInCategory(EventCategory category) const
		{
			return GetCategoryFlags() & category;
		}

	public:
		bool Handled = false;
	};
	
	class CGR_API EventListenerInterface
	{
		friend class EventManager;

	public:
		virtual ~EventListenerInterface() { }

		void Exec(Event& e)
		{
			Call(e);
		}

		void SetHandled(Event& e) { e.Handled = true; }
		bool GetIsHandled(Event& e) { return e.Handled; }

		virtual bool IsActive() = 0;

	private:
		virtual void Call(Event& e) = 0;

	};

	template<typename T>
	class CGR_API EventListener : public EventListenerInterface
	{
		friend class EventManager;

	public:
		EventListener() {}
		EventListener(const EventCallbackHandle& eventCallback)
			: m_EventCallbackFn(eventCallback)
		{
			CGR_CORE_INFO("Added event listener");
		}

		~EventListener()
		{
			Active = false;
			CGR_CORE_INFO("Removed event listener");
		}

	private:
		virtual void Call(Event& e) override
		{
			if(T::GetStaticType() == e.GetEventType())
			{
				if (!GetIsHandled(e))
				{
					m_EventCallbackFn(e);
					SetHandled(e);
				}
			}
		}

		virtual bool IsActive() override { return Active; }

		EventCallbackHandle m_EventCallbackFn;
		bool Active = true;
	};

	class EventManager
	{
	public:
		EventManager() = default;
		EventManager(LayerStack* layerStack)
			: m_LayerStack(layerStack) {}

		template<typename T>
		// Listen to events independent of application layer
		void AddGlobalListener(const EventCallbackHandle& eventHandle)
		{
			m_EventQueue.push_back(CreateScope<EventListener<T>>(eventHandle));
		}

		// Dispatches layer independent events
		void DispatchGlobalEvents(Event& e)
		{
			for (auto it = m_EventQueue.begin(); it != m_EventQueue.end(); it++)
			{
				if (it->get()->IsActive())
				{
						it->get()->Call(e);
						if (e.Handled)
							break;
				}
				else
				{
					m_EventQueue.erase(it);
				}
			}

			for (auto it = m_LayerStack->end(); it != m_LayerStack->begin();)
			{
				(*--it)->OnEvent(e);
				if (e.Handled)
					break;
			}
		}

	private:
		std::vector<Scope<EventListenerInterface>> m_EventQueue;
		LayerStack* m_LayerStack;
	};

	class CGR_API EventDispatcher
	{
	public:
		EventDispatcher(Event& e)
			: m_Event(e) {}

		template<typename T, typename F>
		void Dispatch(const F& func)
		{
			if (T::GetStaticType() == m_Event.GetEventType())
			{
				if (!m_Event.Handled)
				{
					// TODO: event functions must return a boolean
					func(static_cast<T&>(m_Event));
					//m_Event.Handled = true;
				}
			}
		}

	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, Event& e)
	{
		return os << e.ToString();
	}
}