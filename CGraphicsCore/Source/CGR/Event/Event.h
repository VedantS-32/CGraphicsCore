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

	using EventCallbackFn = std::function<void(const Event&)>;

	enum class EventType
	{
		None = 0,
		WindowResized, WindowMoved, WindowFocused, WindowLostFocus, WindowClosed,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseButtonClicked, MouseMoved, MouseScrolled
	};

	class CGR_API Event
	{
		friend class EventManager;
		friend class EventListenerInterface;
	public:
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual std::string ToString() const = 0;

	private:
		mutable bool Handled = false;
	};
	
	class CGR_API EventListenerInterface
	{
		friend class EventManager;

	public:
		virtual ~EventListenerInterface() { }

		void Exec(const Event& e)
		{
			Call(e);
		}

		void SetHandled(const Event& e) { e.Handled = true; }
		bool GetIsHandled(const Event& e) { return e.Handled; }

		virtual bool IsActive() = 0;

	private:
		virtual void Call(const Event& e) = 0;

	};

	template<typename T>
	class CGR_API EventListener : public EventListenerInterface
	{
		friend class EventManager;

	public:
		EventListener() {}
		EventListener(const EventCallbackFn& eventCallback)
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
		virtual void Call(const Event& e) override
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

		EventCallbackFn m_EventCallbackFn;
		bool Active = true;
	};

	class EventManager
	{
	public:
		template<typename T>
		void AddListener(const EventCallbackFn& eventHandle)
		{
			m_EventQueue.push_back(CreateScope<EventListener<T>>(eventHandle));
		}

		void SetLayerStack(LayerStack* layerStack) { m_LayerStack = layerStack; }

		void Dispatch(const Event& e)
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
				if (e.Handled) break;
			}
		}

	private:
		std::vector<Scope<EventListenerInterface>> m_EventQueue;
		LayerStack* m_LayerStack;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}