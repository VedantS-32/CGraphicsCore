require "Script/PremakeVSCode"

workspace "CGraphicsCore"
	architecture "x64"
	startproject "CGraphicsSandbox"

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
	IncludeDir["imgui"] = "%{wks.location}/CGraphicsCore/Vendor/imgui"
	IncludeDir["ImGuizmo"] = "%{wks.location}/CGraphicsCore/Vendor/ImGuizmo"
	IncludeDir["stb_image"] = "%{wks.location}/CGraphicsCore/Vendor/stb_image"
	IncludeDir["assimp"] = "%{wks.location}/CGraphicsCore/Vendor/assimp/include"
	IncludeDir["yaml_cpp"] = "%{wks.location}/CGraphicsCore/Vendor/yaml-cpp/include"
	IncludeDir["entt"] = "%{wks.location}/CGraphicsCore/Vendor/entt/include"
	IncludeDir["hashlink"] = "%{wks.location}/CGraphicsCore/Vendor/hashlink/src"

	group "Dependencies"
		include "CGraphicsCore/Vendor/glfw"
		include "CGraphicsCore/Vendor/ImGui"
		include "CGraphicsCore/Vendor/ImGuizmo"
		include "CGraphicsCore/Vendor/assimp"
		include "CGraphicsCore/Vendor/yaml-cpp"
		include "CGraphicsCore/Vendor/hashlink"
	group ""

include "CGraphicsCore"
include "CGraphicsSandbox"
include "CGraphicsScript"