project "CGraphicsScript"
	location "%{wks.location}/CGraphicsScript"
	kind "SharedLib"
	targetextension ".hdll"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	flags { "MultiProcessorCompile" }

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	defines {
		"CGR_DYNAMIC_LINK"
	}

	includedirs {
		"Source",
		"%{wks.location}/CGraphicsCore/Source",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.hashlink}"
	}

	files {
        "Source/**.h",
        "Source/**.cpp",
		"Source/**.hx"
	}

	links {
		"CGraphicsCore",
		"libhl"
	}

	filter "action:vs*"
		buildoptions { "/utf-8" }
		postbuildcommands {
			("{MKDIR} %{wks.location}/bin/" .. outputdir .. "/CGraphicsSandbox"),
			("{COPY} %{cfg.buildtarget.relpath} %{wks.location}/bin/" .. outputdir .. "/CGraphicsSandbox")
		}
	
	filter "action:not vs*"
		postbuildcommands {
			("mkdir -p \"%{wks.location}/bin/" .. outputdir .. "/CGraphicsSandbox\"|| exit 0"),
			("cp -f %{cfg.buildtarget.relpath} %{wks.location}/bin/" .. outputdir .. "/CGraphicsSandbox")
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
