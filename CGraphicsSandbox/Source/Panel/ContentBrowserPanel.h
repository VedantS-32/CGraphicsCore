#pragma once

#include <CGR.h>

#include <filesystem>

namespace Cgr
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();

	private:
		std::filesystem::path m_CurrentDirectory;

		Ref<Texture2D> m_DirectoryIcon;
		AssetHandle m_DirectoryIconHandle;
		Ref<Texture2D> m_FileIcon;
		AssetHandle m_FileIconHandle;
		AssetHandle m_Handle;
	};
}