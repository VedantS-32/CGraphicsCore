#include "CGRpch.h"
#include "ScriptEngine.h"

#include <cwchar>

namespace Cgr
{
    static bool hl_is_valid_ptr(void* p) {
        if (!p) return false;
#ifdef _WIN32
        MEMORY_BASIC_INFORMATION mbi;
        return VirtualQuery(p, &mbi, sizeof(mbi)) &&
            (mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE));
#else
        return msync(p, 1, MS_ASYNC) == 0; // Rough check
#endif
    }

    static void PrintAllFunctions(hl_module* module) {
        if (!module || !module->code) return;

        std::cout << "=== Haxe Functions ===" << std::endl;
        for (int i = 0; i < module->code->nfunctions; ++i) {
            hl_function* func = &module->code->functions[i];

            // Get the most specific name available
            const uchar* name = nullptr;
            if (func->obj) {
                name = func->field.name;
            }
            else if (func->field.ref) {
                name = func->field.ref->field.name;
            }

            // Print function index and type
            std::cout << "[" << func->findex << "] ";
            if (func->type) {
                uchar* typeStr = const_cast<uchar*>(hl_type_str(func->type));
                std::cout << "Type: " << hl_to_utf8(typeStr) << " ";
            }

            // Print name if available
            if (name) {
                char* cname = hl_to_utf8(name);
                std::cout << "Name: " << cname;
            }
            else {
                std::cout << "[ANONYMOUS]";
            }

            std::cout << std::endl;
        }
    }

	ScriptEngine::~ScriptEngine()
	{
		if (m_Initialized) {
			hl_global_free();
		}
	}

	bool ScriptEngine::Init()
	{
		if (m_Initialized) return true;

		// Initialize HashLink globals
		hl_global_init();
		m_Initialized = true;
		return true;
	}

	void ScriptEngine::Shutdown()
	{
	}

	bool ScriptEngine::LoadScript(const std::string& scriptPath)
    {
        // Load bytecode file
        FILE* f = fopen(scriptPath.c_str(), "rb");
        if (!f) {
            CGR_CORE_ERROR("Failed to open HashLink bytecode file: {}", scriptPath);
            return false;
        }

        fseek(f, 0, SEEK_END);
        int size = (int)ftell(f);
        fseek(f, 0, SEEK_SET);

        unsigned char* data = (unsigned char*)malloc(size);
        if (!data) {
            fclose(f);
            return false;
        }

        if (fread(data, 1, size, f) != size) {
            CGR_CORE_ERROR("Failed to read file: {}", scriptPath);
            free(data);
            fclose(f);
            return false;
        }
        fclose(f);

        // Parse bytecode
        char* errorMsg = nullptr;
        hl_code* code = hl_code_read(data, size, &errorMsg);
        free(data);

        if (!code) {
            CGR_CORE_ERROR("HL bytecode error: {}", errorMsg ? errorMsg : "unknown");
            if (errorMsg) free(errorMsg);
            return false;
        }

        // Create module
        hl_module* module = hl_module_alloc(code);
        if (!module) {
            CGR_CORE_ERROR("Failed to create HL module");
            hl_code_free(code);
            return false;
        }

        // Critical initialization step
        if (hl_module_init(module, false, false) == 0) {
            CGR_CORE_ERROR("Failed to initialize HL module");
            hl_module_free(module);
            hl_code_free(code);
            return false;
        }

        // Force JIT compilation of all functions
#ifdef HL_JIT
        if (!hl_jit_all_functions(module)) {
            CGR_CORE_WARN("JIT compilation failed for some functions");
        }
#endif

        // Store references
        m_Module = module;
        m_Code = code; // Keep code reference
        m_CachedFunctions.clear();

        // Initialize sys args
        m_Args.clear();
        m_Args.push_back(nullptr); // null terminator
        hl_sys_init(m_Args.data(), (int)m_Args.size(), nullptr);

        return true;
    }


    vclosure* ScriptEngine::GetFunction(const std::string& functionPath)
    {
        auto it = m_CachedFunctions.find(functionPath);
        if (it != m_CachedFunctions.end()) return it->second;

        if (!m_Module || !m_Module->code) {
            CGR_CORE_ERROR("Module not loaded");
            return nullptr;
        }

        //PrintAllFunctions(m_Module);

        // Convert to HL's internal string format
        uchar* searchName = hl_to_utf16(functionPath.c_str());
        int searchHash = hl_hash_gen(searchName, true);

        // Search through all functions using HL's metadata
        for (int i = 0; i < m_Module->code->nfunctions; ++i) {
            hl_function* func = &m_Module->code->functions[i];

            // Get function name from proper source
            const uchar* name = nullptr;
            if (func->obj) {
                name = func->field.name;
            }
            else if (func->field.ref) {
                name = func->field.ref->field.name;
            }
            if (!name) continue;

            // Compare hashes first for efficiency
            if (hl_hash_gen(name, false) != searchHash) continue;

            // Direct UTF-16 comparison
            if (std::wcscmp(name, searchName) != 0) continue;

            // Verify function type
            if (!func->type || func->type->kind != HFUN) {
                CGR_CORE_WARN("Found matching name but invalid function type");
                continue;
            }

            // Get the actual function index
            int findex = func->findex;
            //if (i < 0 || i >= m_Module->code->nfunctions) {
            //    CGR_CORE_ERROR("Invalid function index {}", findex);
            //    continue;
            //}

            // Get closure from JIT-compiled pointers
            vclosure* closure = hl_alloc_closure_void(func->type, m_Module->functions_ptrs[findex]);

            // Validate closure structure
            if (!hl_is_valid_ptr(closure)) { // Custom memory validation
                CGR_CORE_ERROR("Invalid closure pointer at index {0}", findex);
                continue;
            }

            // Validate type header
            if (closure->t < (hl_type*)0x1000 || closure->t->kind < 0 || closure->t->kind >= HLAST) {
                CGR_CORE_ERROR("Corrupted type info for closure at index {0}", findex);
                continue;
            }

            // Validate function type
            if (closure->t->kind != HFUN) {
                CGR_CORE_ERROR("Invalid closure type ... at index {0}", findex);
                continue;
            }

            //CGR_CORE_TRACE("Found function {0} at index {1} (findex {2})", functionPath, i, findex);

            m_CachedFunctions[functionPath] = closure;
            return closure;
        }

        CGR_CORE_ERROR("Function {0} not found in module", functionPath);
        return nullptr;
    }

    void ScriptEngine::CallVoidFunction(const std::string& functionPath, const std::vector<vdynamic*>& args)
    {
        vclosure* closure = GetFunction(functionPath);
        if (!closure)
        {
            CGR_CORE_ASSERT(false, "Couldn't find function from the haxe script");
            return;
        }
        // Call the function, but ignore the return value
        hl_dyn_call(closure, const_cast<vdynamic**>(args.data()), (int)args.size());
    }

    void ScriptEngine::CallVoidFunction(const std::string& functionPath)
    {
        vclosure* closure = GetFunction(functionPath);
        if (!closure)
        {
            CGR_CORE_ASSERT(false, "Couldn't find function from the haxe script");
            return;
        }

        // Call the function with no arguments
        hl_dyn_call(closure, nullptr, 0);
    }

	void ScriptEngine::Update(float deltaTime)
	{
	}
}
