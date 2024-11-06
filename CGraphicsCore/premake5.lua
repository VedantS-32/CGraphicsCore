project "ConstellationCore"
	location "ConstellationCore"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	flags { "MultiProcessorCompile" }

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	pchheader "CGraphicspch.h"
	pchsource "%{prj.name}/Source/CGraphicspch.cpp"

	files {
		"%{prj.name}/Source/**.h",
		"%{prj.name}/Source/**.cpp",
		"%{prj.name}/Vendor/glm/**.h",
		"%{prj.name}/Vendor/glm/**.hpp",
		"%{prj.name}/Vendor/glm/**.inl",
		"%{prj.name}/Vendor/stb_image/**.h",
		"%{prj.name}/Vendor/stb_image/**.cpp",
	}

	includedirs {
		"%{prj.name}/Source",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glfw}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.assimp}"
	}

	links {
		"glfw",
		"Glad",
		"imgui",
		"ImGuizmo",
		"yaml-cpp",
		"assimp"
	}

	postbuildcommands {
        ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/CGraphicsObservatory")
    }

	filter "system:windows"
		systemversion "latest"
		defines {
			"CSTELL_PLATFORM_WINDOWS",
			"CSTELL_DYNAMIC_LINK",
			"CSTELL_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}
		links {
			"Gdi32.lib",
			"User32.lib",
			"Shell32.lib",
			"Comdlg32.lib",
			"opengl32.lib"
		}
	
	filter "system:linux"
		defines {
			"CSTELL_PLATFORM_LINUX",
			"CSTELL_DYNAMIC_LINK",
			"CSTELL_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

	filter "system:macosx"
		defines {
			"CSTELL_PLATFORM_MACOSX",
			"CSTELL_DYNAMIC_LINK",
			"CSTELL_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

	filter "files:ConstellationCore/Vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }

	filter "configurations:Debug"
		defines "CSTELL_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "CSTELL_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "CSTELL_DIST"
		runtime "Release"
		optimize "On"