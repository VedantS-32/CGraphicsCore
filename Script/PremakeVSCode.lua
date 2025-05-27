newoption {
    trigger = "jobs",
    value = "NUMBER",
    description = "Number of parallel jobs for make",
    default = "4"
}

newaction {
    trigger = "vscode",
    description = "Generate VS Code configuration files",
    
    onWorkspace = function(wks)
        -- Create .vscode directory
        os.mkdir(".vscode")
        
        -- Generate tasks.json
        generateTasks(wks)
        
        -- Generate c_cpp_properties.json
        generateCppProperties(wks)
        
        -- Generate launch.json
        generateLaunch(wks)
        
        print("VS Code configuration files generated!")
    end
}

function getCpuCores()
    local cores = 4  -- default fallback
    
    if os.target() == "windows" then
        local handle = io.popen("echo %NUMBER_OF_PROCESSORS%")
        if handle then
            local result = handle:read("*a")
            handle:close()
            cores = tonumber(result:match("%d+")) or cores
        end
    else
        local handle = io.popen("nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4")
        if handle then
            local result = handle:read("*a")
            handle:close()
            cores = tonumber(result:match("%d+")) or cores
        end
    end
    
    return tostring(cores)
end

function generateTasks(wks)
    local tasks = {
        version = "2.0.0",
        tasks = {}
    }
    
    -- Generate project files task
    table.insert(tasks.tasks, {
        label = "Generate Project Files",
        type = "shell",
        command = "premake5",
        args = {"gmake2"},
        group = "build",
        presentation = {
            echo = true,
            reveal = "always",
            focus = false,
            panel = "shared"
        },
        problemMatcher = {}
    })
    
    local jobs = _OPTIONS["jobs"] or getCpuCores()
    
    -- Build tasks for each configuration
    for _, cfg in ipairs(wks.configurations) do
        local configLower = string.lower(cfg)
        
        table.insert(tasks.tasks, {
            label = "Build " .. cfg,
            type = "shell",
            command = "make",
            args = {"-j" .. jobs, " config=" .. configLower},
            group = cfg == "Debug" and {kind = "build", isDefault = true} or "build",
            presentation = {
                echo = true,
                reveal = "always",
                focus = false,
                panel = "shared"
            },
            problemMatcher = {"$gcc"},
            dependsOn = "Generate Project Files"
        })
    end
    
    -- Clean task
    table.insert(tasks.tasks, {
        label = "Clean",
        type = "shell",
        command = "make",
        args = {"clean"},
        group = "build"
    })
    
    -- Write tasks.json
    local file = io.open(".vscode/tasks.json", "w")
    file:write(json.encode_pretty(tasks))
    file:close()
end

function generateCppProperties(wks)
    local properties = {
        configurations = {},
        version = 4
    }
    
    -- Get include paths from first project (you can modify this logic)
    local firstProject = wks.projects[1]
    local includePaths = {"${workspaceFolder}/**"}
    
    if firstProject then
        for _, incdir in ipairs(firstProject.includedirs) do
            table.insert(includePaths, "${workspaceFolder}/" .. incdir)
        end
    end
    
    -- Windows configuration
    table.insert(properties.configurations, {
        name = "Win32",
        includePath = includePaths,
        defines = {"_DEBUG", "UNICODE", "_UNICODE"},
        compilerPath = "C:/mingw64/bin/gcc.exe",
        cStandard = "c17",
        cppStandard = "c++17",
        intelliSenseMode = "windows-gcc-x64"
    })
    
    -- Linux configuration
    table.insert(properties.configurations, {
        name = "Linux",
        includePath = includePaths,
        defines = {},
        compilerPath = "/usr/bin/gcc",
        cStandard = "c17",
        cppStandard = "c++17",
        intelliSenseMode = "linux-gcc-x64",
        compileCommands = "${workspaceFolder}/compile_commands.json"
    })
    
    -- Mac configuration
    table.insert(properties.configurations, {
        name = "Mac",
        includePath = includePaths,
        defines = {},
        compilerPath = "/usr/bin/clang",
        cStandard = "c17",
        cppStandard = "c++17",
        intelliSenseMode = "macos-clang-x64"
    })
    
    -- Write c_cpp_properties.json
    local file = io.open(".vscode/c_cpp_properties.json", "w")
    file:write(json.encode_pretty(properties))
    file:close()
end

function generateLaunch(wks)
    local launch = {
        version = "0.2.0",
        configurations = {}
    }
   local arch = io.popen("uname -m 2>/dev/null || echo x86_64"):read("*a"):gsub("%s+", "")
    
    -- Generate launch configs for executable projects
    for _, prj in ipairs(wks.projects) do
        if prj.kind == "ConsoleApp" or prj.kind == "WindowedApp" then
            for _, cfg in ipairs(wks.configurations) do
                local configLower = string.lower(cfg)
                
                local binaryPath = string.format("${workspaceFolder}/bin/%s-%s-%s/%s/%s", cfg, os.target(), arch, prj.name, prj.name)

                -- Add .exe extension on Windows
                if os.target() == "windows" then
                    binaryPath = binaryPath .. ".exe"
                end
                
                table.insert(launch.configurations, {
                    name = prj.name .. " (" .. cfg .. ")",
                    type = "cppdbg",
                    request = "launch",
                    program = binaryPath,
                    args = {},
                    stopAtEntry = false,
                    cwd = "${workspaceFolder}/".. prj.name,
                    environment = {},
                    console = true,
                    MIMode = os.target() == "windows" and "gdb" or "gdb",
                    preLaunchTask = "Build " .. cfg
                })

                table.insert(launch.configurations, {
                    name = "Launch " .. prj.name .. " (" .. cfg .. ")",
                    type = "cppvsdbg",
                    request = "launch",
                    program = binaryPath,
                    args = {},
                    stopAtEntry = false,
                    cwd = "${workspaceFolder}/".. prj.name,
                    environment = {},
                    console = true,
                    MIMode = os.target() == "windows" and "gdb" or "gdb",
                    preLaunchTask = "Build " .. cfg
                })
            end
        end
    end
    
    -- Write launch.json
    local file = io.open(".vscode/launch.json", "w")
    file:write(json.encode_pretty(launch))
    file:close()
end

-- JSON encoding functions (simple implementation)
json = {}

function json.encode_pretty(obj)
    return json._encode_pretty(obj, 0)
end

function json._encode_pretty(obj, indent)
    local indent_str = string.rep("  ", indent)
    local next_indent_str = string.rep("  ", indent + 1)
    
    if type(obj) == "table" then
        -- Check if it's an array (has numeric indices starting from 1)
        local is_array = true
        local max_index = 0
        for k, v in pairs(obj) do
            if type(k) ~= "number" or k <= 0 or k ~= math.floor(k) then
                is_array = false
                break
            end
            max_index = math.max(max_index, k)
        end
        
        if is_array and max_index > 0 then
            -- Array with elements
            local result = "[\n"
            for i = 1, max_index do
                if obj[i] ~= nil then
                    result = result .. next_indent_str .. json._encode_pretty(obj[i], indent + 1)
                    if i < max_index then result = result .. "," end
                    result = result .. "\n"
                end
            end
            result = result .. indent_str .. "]"
            return result
        elseif is_array then
            -- Empty array
            return "[]"
        else
            -- Object
            local result = "{\n"
            local first = true
            for k, v in pairs(obj) do
                if not first then result = result .. ",\n" end
                result = result .. next_indent_str .. '"' .. tostring(k) .. '": ' .. json._encode_pretty(v, indent + 1)
                first = false
            end
            result = result .. "\n" .. indent_str .. "}"
            return result
        end
    elseif type(obj) == "string" then
        return '"' .. obj:gsub('"', '\\"'):gsub('\\', '\\\\') .. '"'
    elseif type(obj) == "boolean" then
        return obj and "true" or "false"
    elseif type(obj) == "number" then
        return tostring(obj)
    else
        return "null"
    end
end