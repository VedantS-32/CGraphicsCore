#pragma once

#include "hl.h"
#include "HashlinkHelper.h"

#include "CGR/Core/Core.h"
#include "CGR/Scene/Entity.h"

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

        void CreateEntityInstance(Entity& entity);

        void OnEntityBegin(Entity& entity);
		void OnEntityUpdate(Entity& entity, Timestep ts);

        // Attempts to get the function from the hl bytecode
        vclosure* GetFunction(const std::string& functionPath);
        vclosure* GetMethodClosure(hl_obj_field& field);

        void CallVoidFunction(const std::string& functionPath, const std::vector<vdynamic*>& args);
        void CallVoidFunction(const std::string& functionPath);

    private:
        hl_module* m_Module = nullptr;
        hl_code* m_Code = nullptr;
        std::vector<void*> m_Args;
        bool m_Initialized = false;
        std::unordered_map<std::string, vclosure*> m_CachedFunctions;
        std::unordered_map<EntityHandle, vdynamic*> m_CachedObjects;
	};
}