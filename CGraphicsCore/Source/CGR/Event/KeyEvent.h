#pragma once

#include "Event.h"
#include "CGR/Core/KeyCode.h"
#include "CGRpch.h"

namespace Cgr
{
	class KeyEvent : public Event
	{
	public:
		KeyEvent(KeyCode key)
			: m_KeyCode(key) {}

		KeyCode GetKey() const { return m_KeyCode; }
		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)


	protected:
		KeyCode m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(KeyCode key, int repeatCount)
			: KeyEvent(key), m_RepeatCount(repeatCount) {}

		inline int GetRepeatCount() const { return m_RepeatCount; }

		virtual std::string ToString() const
		{
			std::stringstream ss;
			ss << "Key pressed event: " << m_KeyCode << ", RepeatCount: " << m_RepeatCount;

			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed);

	private:
		int m_RepeatCount;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(KeyCode key)
			: KeyEvent(key) {}

		virtual std::string ToString() const
		{
			std::stringstream ss;
			ss << "Key released event: " << m_KeyCode;

			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased);
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(KeyCode key)
			: KeyEvent(key) {}

		virtual std::string ToString() const
		{
			std::stringstream ss;
			ss << "Key typed event: " << m_KeyCode;

			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped);
	};

}