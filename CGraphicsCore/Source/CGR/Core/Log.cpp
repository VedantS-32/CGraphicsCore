#include "CGRpch.h"

#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Cgr
{
    Ref<spdlog::logger> Log::s_CoreLogger = nullptr;
    Ref<spdlog::logger> Log::s_ClientLogger = nullptr;

    const Ref<spdlog::logger>& Cgr::Log::GetCoreLogger()
    {
        return s_CoreLogger;
    }

    const Ref<spdlog::logger>& Cgr::Log::GetClientLogger()
    {
        return s_ClientLogger;
    }

    void Log::Init()
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");
        s_CoreLogger = spdlog::stdout_color_mt("CORE");
        s_CoreLogger->set_level(spdlog::level::trace);

        s_ClientLogger = spdlog::stdout_color_mt("APP");
        s_ClientLogger->set_level(spdlog::level::trace);
    }
}