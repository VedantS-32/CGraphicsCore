#include "ContentBrowserPanel.h"

#include <CGR.h>
#include <imgui.h>

namespace Cgr
{
	extern const std::filesystem::path s_AssetPath = "Content";

	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentDirectory(s_AssetPath)
	{
		auto assetManager = Application::Get().GetAssetManager();

		auto handle = assetManager->ImportAsset("Content/Icon/ContentBrowser/Folder.png");
		m_IconMap["Folder"] = assetManager->GetAsset<Texture2D>(handle);
		handle = assetManager->ImportAsset("Content/Icon/ContentBrowser/File.png");
		m_IconMap["File"] = assetManager->GetAsset<Texture2D>(handle);
		handle = assetManager->ImportAsset("Content/Icon/ContentBrowser/Shader.png");
		m_IconMap["Shader"] = assetManager->GetAsset<Texture2D>(handle);
		handle = assetManager->ImportAsset("Content/Icon/ContentBrowser/Material.png");
		m_IconMap["Material"] = assetManager->GetAsset<Texture2D>(handle);
		handle = assetManager->ImportAsset("Content/Icon/ContentBrowser/Model.png");
		m_IconMap["Model"] = assetManager->GetAsset<Texture2D>(handle);

		m_Handle = m_IconMap["Folder"]->Handle;
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		if (m_CurrentDirectory != std::filesystem::path(s_AssetPath))
		{
			if (ImGui::Button("<-"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}

		static float padding = 16.0f;
		static float thumbnailSize = 115.0f;
		static float dragDropPreviewSize = 64.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();

			auto relativePath = std::filesystem::relative(directoryEntry.path(), s_AssetPath);
			std::string filenameString = relativePath.filename().string();
			ImGui::PushID(filenameString.c_str());

			auto extension = directoryEntry.path().filename().extension().string();
			std::filesystem::path assetPath = std::filesystem::path(s_AssetPath) / relativePath;

			Ref<Texture2D> icon;
			auto assetManager = Application::Get().GetAssetManager();

			if (directoryEntry.is_directory())
			{
				icon = m_IconMap["Folder"];
				m_Handle = icon->Handle;
			}
			else
			{
				if(extension == ".csmat")
				{
					assetManager->ImportAsset(assetPath);
					icon = m_IconMap["Material"];
					m_Handle = icon->Handle;
				}
				else if(extension == ".glsl")
				{
					assetManager->ImportAsset(assetPath);
					icon = m_IconMap["Shader"];
					m_Handle = icon->Handle;
				}
				else if(extension == ".csmesh" || extension == ".obj" || extension == ".fbx")
				{
					assetManager->ImportAsset(assetPath);
					icon = m_IconMap["Model"];
					m_Handle = icon->Handle;
				}
				else if(extension == ".png" || extension == ".jpg")
				{
					m_Handle = assetManager->ImportAsset(assetPath);
					icon = assetManager->GetAsset<Texture2D>(m_Handle);
				}
				else
				{
					icon = m_IconMap["File"];
					m_Handle = icon->Handle;
				}
			}
			ImGui::ImageButton("FilePreview", reinterpret_cast<void*>(static_cast<uintptr_t>(icon->GetRendererID())), {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});

			auto assetHandle = assetManager->GetAssetHandleFromRegistry(assetPath);
			if (assetHandle)
			{
				if (ImGui::BeginDragDropSource())
				{
					ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", assetHandle.ValuePtr(), sizeof(uint64_t), ImGuiCond_Once);
					ImGui::Image(reinterpret_cast<void*>(static_cast<uintptr_t>(icon->GetRendererID())), { dragDropPreviewSize, dragDropPreviewSize }, { 0, 1 }, { 1, 0 });
					ImGui::EndDragDropSource();
				}
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory())
					m_CurrentDirectory /= path.filename();
			}

			ImGui::TextWrapped(filenameString.c_str());

			ImGui::NextColumn();
			ImGui::PopID();
		}

		ImGui::Columns(1);

		ImGui::PopStyleColor();
		ImGui::End();
	}
}
