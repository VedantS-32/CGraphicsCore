#pragma once

#include "CGR/Core/Core.h"

#include "hl.h"
#include "HashlinkHelper.h"

namespace Cgr
{
	class CGR_API ScriptEngine
	{
    public:
        ScriptEngine() = default;
        ~ScriptEngine();
        bool Init();
        void Shutdown();

        // Load a Haxe script by class name
        bool LoadScript(const std::string& scriptPath);

        // Attempts to get the function from the hl bytecode
        vclosure* GetFunction(const std::string& functionPath);

        void CallVoidFunction(const std::string& functionPath, const std::vector<vdynamic*>& args);
        void CallVoidFunction(const std::string& functionPath);

        // Update method to be called each frame
        void Update(float deltaTime);

    private:
        hl_module* m_Module = nullptr;
        hl_code* m_Code = nullptr;
        std::vector<void*> m_Args;
        bool m_Initialized = false;
        std::unordered_map<std::string, vclosure*> m_CachedFunctions;
	};
}