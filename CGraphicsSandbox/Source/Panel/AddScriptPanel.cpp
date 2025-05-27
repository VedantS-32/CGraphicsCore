#include "AddScriptPanel.h"

#include "misc/cpp/imgui_stdlib.h"

namespace Cgr
{
    static std::string s_ScriptName = "";

    void AddScriptPanel::OpenAddScriptPanel()
    {
        ImGui::OpenPopup("Add Script");
        s_ScriptName = "";
    }

    bool AddScriptPanel::OnImGuiRender(std::string& scriptPath)
	{
        // Get window size for centering
        ImVec2 center = ImGui::GetIO().DisplaySize;
        center.x *= 0.5f;
        center.y *= 0.5f;

        bool toAdd = false;

        // Set the popup position
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
        if (ImGui::BeginPopupModal("Add Script", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Enter the script path:");

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 3));
			if (ImGui::InputText("Script", &s_ScriptName))
			{

			}

			ImGui::SameLine();
            if (ImGui::Button("Browse"))
            {
                std::string filepath = FileDialogs::OpenFile("Haxe script (*.hx)\0*.hx\0");
                if (!filepath.empty())
                {
                    s_ScriptName = filepath;
                }
            }

            ImVec4 playButton = { 0.7f, 0.2f, 0.2f, 1.0f };
            ImVec4 playButtonHovered = { 0.8f, 0.3f, 0.3f, 1.0f };

            ImGui::PushStyleColor(ImGuiCol_Button, playButton);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, playButtonHovered);

            //float menuBarWidth = ImGui::CalcItemWidth();
            //ImVec2 originalPos = ImGui::GetCursorPos();
            //ImGui::SetCursorPosX(menuBarWidth);

            if (ImGui::Button("Cancel"))
            {
                toAdd = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::PopStyleColor(2);

            playButton = { 0.2f, 0.7f, 0.3f, 1.0f };
            playButtonHovered = { 0.3f, 0.8f, 0.4f, 1.0f };

            ImGui::PushStyleColor(ImGuiCol_Button, playButton);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, playButtonHovered);
            
            ImGui::SameLine(0.0f);
            if (ImGui::Button("Add"))
            {
				if (s_ScriptName.empty())
				{
					CGR_CORE_ERROR("Script path is empty");
					toAdd = false;
				}
                else
                {
                    scriptPath = s_ScriptName;
                    toAdd = true;
                }
                ImGui::CloseCurrentPopup();
            }

			ImGui::PopStyleColor(2);
            ImGui::PopStyleVar();

			//ImGui::SetCursorPos(originalPos);

            ImGui::EndPopup();
        }

        ImGui::PopStyleVar();
        return toAdd;
	}
}
