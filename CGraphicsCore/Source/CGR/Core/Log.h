#pragma once

#include "Core.h"

#include "spdlog/spdlog.h"

namespace Cgr
{
	class CGR_API Log
	{
	public:
		static void Init();

		static const Ref<spdlog::logger>& GetCoreLogger();
		static const Ref<spdlog::logger>& GetClientLogger();

	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
	};
}

//Core log macros
#define CGR_CORE_TRACE(...)	::Cgr::Log::GetCoreLogger()->trace(__VA_ARGS__);
#define CGR_CORE_INFO(...)	::Cgr::Log::GetCoreLogger()->info(__VA_ARGS__);
#define CGR_CORE_WARN(...)	::Cgr::Log::GetCoreLogger()->warn(__VA_ARGS__);
#define CGR_CORE_ERROR(...)	::Cgr::Log::GetCoreLogger()->error(__VA_ARGS__);
#define CGR_CORE_FATAL(...)	::Cgr::Log::GetCoreLogger()->fatal(__VA_ARGS__);
//Client log macros
#define CGR_TRACE(...)	::Cgr::Log::GetClientLogger()->trace(__VA_ARGS__);
#define CGR_INFO(...)	::Cgr::Log::GetClientLogger()->info(__VA_ARGS__);
#define CGR_WARN(...)	::Cgr::Log::GetClientLogger()->warn(__VA_ARGS__);
#define CGR_ERROR(...)	::Cgr::Log::GetClientLogger()->error(__VA_ARGS__);
#define CGR_FATAL(...)	::Cgr::Log::GetClientLogger()->fatal(__VA_ARGS__);