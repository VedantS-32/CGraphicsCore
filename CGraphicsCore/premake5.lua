project "CGraphicsCore"
	location "%{wks.location}/CGraphicsCore"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	flags { "MultiProcessorCompile" }

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "CGRpch.h"
	pchsource "Source/CGRpch.cpp"

	files {
		"Source/**.h",
		"Source/**.cpp",

		"Vendor/glad/include/glad/glad.h",
		"Vendor/glad/src/glad.c",
		"Vendor/glad/include/KHR/khrplatform.h",
		"Vendor/spdlog/**.h",
		"Vendor/glm/**.h",
		"Vendor/glm/**.hpp",
		"Vendor/glm/**.inl",
		"Vendor/stb_image/**.h",
		"Vendor/stb_image/**.cpp"
	}

	includedirs {
		"Source",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glfw}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.yaml_cpp}"
	}

	links {
		"glfw",
		"imgui",
		"ImGuizmo",
		"assimp",
		"yaml-cpp"
	}

	postbuildcommands {
        ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/CGraphicsSandbox")
    }

	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8" }
		defines {
			"CGR_PLATFORM_WINDOWS",
			"CGR_DYNAMIC_LINK",
			"CGR_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
			"YAML_CPP_STATIC_DEFINE"
		}
		links {
			"Gdi32",
			"User32",
			"Shell32",
			"Comdlg32",
			"opengl32"
		}
	
	filter "system:linux"
		defines {
			"CGR_PLATFORM_LINUX",
			"CGR_DYNAMIC_LINK",
			"CGR_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
			"YAML_CPP_STATIC_DEFINE"
		}

	filter "system:macosx"
		defines {
			"CGR_PLATFORM_MACOSX",
			"CGR_DYNAMIC_LINK",
			"CGR_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
			"YAML_CPP_STATIC_DEFINE"
		}

	filter "files:%{prj.name}/Vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }

	filter "files:Vendor/glad/src/glad.c"
		flags { "NoPCH" }

	filter "configurations:Debug"
		defines "CGR_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "CGR_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "CGR_DIST"
		runtime "Release"
		optimize "On"