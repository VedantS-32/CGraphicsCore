#pragma once
#pragma warning(disable : 4251)

#include <memory>
#include <filesystem>

#ifndef CGR_API
	#if defined(CGR_DYNAMIC_LINK)
		#ifdef _MSC_VER // MSVC Compiler
			#ifdef CGR_BUILD_DLL
				#define CGR_API __declspec(dllexport)
			#else
				#define CGR_API __declspec(dllimport)
			#endif
		#else // GCC / Clang
			#ifdef CGR_BUILD_DLL
				#define CGR_API __attribute__((visibility("default")))
			#else
				#define CGR_API  // No import attribute needed for shared lib usage
			#endif
		#endif
	#else
		#define CGR_API // Static linking (empty definition)
	#endif
#endif


#ifdef CGR_DEBUG
#define CGR_ENABLE_ASSERTS
#if defined(CGR_PLATFORM_WINDOWS)
#define CGR_DEBUGBREAK() __debugbreak()
#endif
#endif

#define CGR_EXPAND_MACRO(x) x
#define CGR_STRINGIFY_MACRO(x) #x

#ifdef  CGR_ENABLE_ASSERTS

// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
#define CGR_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { CGR##type##ERROR(msg, __VA_ARGS__); CGR_DEBUGBREAK(); } }
#define CGR_INTERNAL_ASSERT_WITH_MSG(type, check, ...) CGR_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define CGR_INTERNAL_ASSERT_NO_MSG(type, check) CGR_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", CGR_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define CGR_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define CGR_INTERNAL_ASSERT_GET_MACRO(...) CGR_EXPAND_MACRO( CGR_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, CGR_INTERNAL_ASSERT_WITH_MSG, CGR_INTERNAL_ASSERT_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define CGR_ASSERT(...) CGR_EXPAND_MACRO( CGR_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#define CGR_CORE_ASSERT(...) CGR_EXPAND_MACRO( CGR_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )

#else
#define	CGR_ASSERT(...)
#define	CGR_CORE_ASSERT(...)
#endif //  CGR_ENABLE_ASSERTS

#define BIT(x) (1 << x)

#define CGR_BIND_EVENT_FN(fn) [this](auto&&... args)->decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace Cgr
{
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}