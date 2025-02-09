project "CGraphicsSandbox"
	location "%{wks.location}/CGraphicsSandbox"
	dependson { "CGraphicsCore" }
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "Off"
	flags { "MultiProcessorCompile" }

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	defines {
		"CGR_DYNAMIC_LINK"
	}

	files {
        "Source/**.h",
        "Source/**.cpp"
	}

	includedirs {
		"Source",
		"%{wks.location}/CGraphicsCore/Source",
        "%{IncludeDir.spdlog}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.entt}"
	}

	links {
		"ImGuizmo",
		"imgui",
		"CGraphicsCore"
	}

	filter "action:vs*"
	postbuildcommands {
		("{COPY} %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/* ./")
	}

	filter "action:not vs*"
		postbuildcommands {
			("cp -f %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/* ./")
		}

	filter "action:vs*"
		buildoptions { "/utf-8" }

	filter "system:windows"
		systemversion "latest"
		defines {
			"CGR_PLATFORM_WINDOWS"
		}

	filter "system:linux"
		defines {
			"CGR_PLATFORM_LINUX"
		}

	filter "system:macosx"
		defines {
			"CGR_PLATFORM_MACOSX"
		}

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
