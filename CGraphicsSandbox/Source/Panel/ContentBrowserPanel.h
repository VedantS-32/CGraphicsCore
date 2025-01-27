#pragma once

#include <CGR.h>

#include <filesystem>

namespace Cgr
{
	using IconMap = std::map<std::string, Ref<Texture2D>>;
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();
		const IconMap& GetIconMap() const { return m_IconMap; }

	private:
		std::filesystem::path m_CurrentDirectory;
		AssetHandle m_Handle;
		IconMap m_IconMap;
	};
}