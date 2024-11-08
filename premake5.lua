workspace "CGraphicsCore"
	architecture "x64"
	startproject "GraphicsSandbox"

	configurations {
		"Debug",
		"Release"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root directory
IncludeDir = {}
IncludeDir["spdlog"] = "%{wks.location}/CGraphicsCore/Vendor/spdlog/include"
IncludeDir["glfw"] = "%{wks.location}/CGraphicsCore/Vendor/glfw/include"
IncludeDir["glad"] = "%{wks.location}/CGraphicsCore/Vendor/glad/include"
IncludeDir["glm"] = "%{wks.location}/CGraphicsCore/Vendor/glm"
IncludeDir["imgui"] = "%{wks.location}/CGraphicsCore/vendor/imgui"
IncludeDir["ImGuizmo"] = "%{wks.location}/CGraphicsCore/vendor/ImGuizmo"
IncludeDir["stb_image"] = "%{wks.location}/CGraphicsCore/vendor/stb_image"
IncludeDir["assimp"] = "%{wks.location}/CGraphicsCore/vendor/assimp/include"

group "Dependencies"
	include "CGraphicsCore/Vendor/glfw"
	include "CGraphicsCore/Vendor/glad"
	include "CGraphicsCore/vendor/ImGui"
	include "CGraphicsCore/vendor/ImGuizmo"
	include "CGraphicsCore/vendor/assimp"
group ""

include "CGraphicsCore"
include "CGraphicsSandbox"