#include "SceneGraphPanel.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

namespace Cgr
{
	SceneGraphPanel::SceneGraphPanel(const Ref<Scene>& context, ContentBrowserPanel* contentBrowserPanel)
	{
        m_AssetManager = Application::Get().GetAssetManager();
        m_Renderer = Application::Get().GetRenderer();
        m_ContentBrowserPanel = contentBrowserPanel;
        m_DefaultTexture = m_AssetManager->GetAsset<Texture2D>(m_AssetManager->GetDefaultAssetHandle(AssetType::Texture2D));
		SetContext(context);
	}

	void SceneGraphPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
		m_SelectionContext = {};
	}

	void SceneGraphPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Graph");
		m_Context->GetRegistry().view<entt::entity>().each([&](auto entityID)
			{
				Entity entity{ entityID, m_Context.get() };
				DrawEntityNode(entity);
			});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
				m_Context->CreateEntity("Empty Entity");

			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
		if (m_SelectionContext)
		{
			DrawComponents(m_SelectionContext);
		}
		ImGui::End();
	}

	void SceneGraphPanel::SetSelectedEntity(Entity entity)
	{
		m_SelectionContext = entity;
	}

	void SceneGraphPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		
		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened)
		{
			ImGui::TreePop();
		}

		if(entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
		}
	}

	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.85f, 0.1f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.95f, 0.2f, 0.30f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.85f, 0.1f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X"))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.25f, 0.82f, 0.25f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.35f, 0.9f, 0.35f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.25f, 0.82f, 0.25f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y"))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.5f, 0.85f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.65f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.5f, 0.85f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z"))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f);
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

    const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth
        | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

    template<typename T, typename UIFunction>
    static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
    {
        if (entity.HasComponent<T>())
        {
            auto& component = entity.GetComponent<T>();
            ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
            float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

            bool opened = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());

            float padding = 4.0f;
            float buttonSize = lineHeight;

            ImGui::SameLine();
            ImGui::SetCursorPosX(contentRegionAvailable.x - padding * 2.0f);

            if (ImGui::Button("-", ImVec2{ buttonSize, buttonSize }))
            {
                ImGui::OpenPopup("ComponentSettings");
            }
            ImGui::PopStyleVar();

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove Component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (opened)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	void SceneGraphPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());

			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("Add Component");

		if (ImGui::BeginPopup("Add Component"))
		{
			if (ImGui::MenuItem("Model"))
			{
				m_SelectionContext.AddComponent<ModelComponent>();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

        ImGui::Spacing();
        ImGui::Spacing();
		DrawComponent<TransformComponent>("Transform", entity, [&](auto& component)
			{
				DrawVec3Control("Translation", component.Translation);

				glm::vec3 rotation = glm::degrees(component.Rotation);
				DrawVec3Control("Rotation", rotation);
				component.Rotation = glm::radians(rotation);

				DrawVec3Control("Scale", component.Scale, 1.0f);
			});

		DrawComponent<ModelComponent>("Model", entity, [&](auto& component)
			{
                auto model = component.GetModel();

                auto& name = model->Name;
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::Text("Model");
                const auto& iconMap = m_ContentBrowserPanel->GetIconMap();
                ImGui::ImageButton(name.c_str(), reinterpret_cast<void*>(static_cast<uintptr_t>(iconMap.at("Model")->GetRendererID())), { 98, 98 }, { 0, 1 }, { 1, 0 });
                ImGui::PopStyleColor();


                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        AssetHandle handle = *reinterpret_cast<const uint64_t*>(payload->Data);
                        component.SetModel(m_AssetManager->GetAsset<Model>(handle));
                    }
                    ImGui::EndDragDropTarget();
                }

                ImGui::SameLine();
                ImGui::BeginGroup();
                ImGui::Text(name.c_str());
                if (ImGui::Button("Save", { 100.0f, 24.0f }))
                {
                    ModelSerializer serializer(model);
                    serializer.Serialize(m_AssetManager->GetFilePath(model->Handle));
                }

                if (ImGui::Button("Load", { 100.0f, 24.0f }))
                {
                    ModelSerializer serializer(model);
                    serializer.Deserialize(m_AssetManager->GetFilePath(model->Handle));
                    component.RefreshShaderBuffer();
                }
                ImGui::EndGroup();

                std::vector<const char*> meshList;
                auto& meshes = model->GetMeshes();

                ImGui::Text("Materials");
                for (auto& mesh : meshes)
                {
                    if (ImGui::TreeNode(std::format("Material {}", mesh.GetMaterialIndex()).c_str()))
                    {

                        int meshIdx = mesh.GetMaterialIndex();
                        auto material = model->GetMaterial(meshIdx);

                        auto& matName = material->Name;
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                        ImGui::ImageButton(matName.c_str(), reinterpret_cast<void*>(static_cast<uintptr_t>(iconMap.at("Material")->GetRendererID())), { 98, 98 }, { 0, 1 }, { 1, 0 });
                        ImGui::PopStyleColor();

                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                            {
                                AssetHandle handle = *reinterpret_cast<const uint64_t*>(payload->Data);
                                model->SetMaterial(meshIdx, m_AssetManager->GetAsset<Material>(handle));
                            }
                            ImGui::EndDragDropTarget();
                        }

                        ImGui::SameLine();
                        ImGui::BeginGroup();
                        ImGui::Text(matName.c_str());
                        if (ImGui::Button("Save", { 100.0f, 24.0f }))
                        {
                            MaterialSerializer serializer(material);
                            serializer.Serialize(m_AssetManager->GetFilePath(material->Handle));
                        }

                        if (ImGui::Button("Load", { 100.0f, 24.0f }))
                        {
                            MaterialSerializer serializer(material);
                            serializer.Deserialize(m_AssetManager->GetFilePath(material->Handle));
                        }
                        ImGui::EndGroup();

                        auto& shaderName = material->GetShader()->Name;
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                        ImGui::Text("Shader");
                        ImGui::ImageButton(shaderName.c_str(), reinterpret_cast<void*>(static_cast<uintptr_t>(iconMap.at("Shader")->GetRendererID())), { 98, 98 }, { 0, 1 }, { 1, 0 });
                        ImGui::PopStyleColor();

                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                            {
                                AssetHandle handle = *reinterpret_cast<const uint64_t*>(payload->Data);
                                auto shader = m_AssetManager->GetAsset<Shader>(handle);
                                shader->Recompile();
                                m_Renderer->SetShaderBuffer(shader);
                                material->SetShader(shader);
                                shader->ExtractSSBOParameters(material.get());
                                shader->UpdateSSBOParameters(material.get(), m_Renderer->GetSSBO());
                            }
                            ImGui::EndDragDropTarget();
                        }

                        ImGui::SameLine();
                        ImGui::BeginGroup();
                        ImGui::Text(shaderName.c_str());
                        if (ImGui::Button("Recompile", { 100.0f, 24.0f }))
                        {
                            auto shader = material->GetShader();
                            shader->Recompile();
                            m_Renderer->SetShaderBuffer(shader);
                            shader->ExtractSSBOParameters(material.get());
                            shader->UpdateSSBOParameters(material.get(), m_Renderer->GetSSBO());
                        }
                        ImGui::EndGroup();

                        auto& textures = material->GetAllTextures();
                        bool removeTex = false;
                        UUID texuuid;
                        ImGui::Text("Material Parameters");
                        int id = 0;
                        for (auto& [uuid, texture] : textures)
                        {
                            auto& texName = texture->GetName();
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                            ImGui::ImageButton(texName.c_str(), reinterpret_cast<void*>(static_cast<uintptr_t>(texture->GetRendererID())), { 98, 98 }, { 0, 1 }, { 1, 0 });
                            ImGui::PopStyleColor();

                            if (ImGui::BeginDragDropTarget())
                            {
                                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                                {
                                    AssetHandle handle = *reinterpret_cast<const uint64_t*>(payload->Data);
                                    texture = m_AssetManager->GetAsset<Texture2D>(handle);
                                }
                                ImGui::EndDragDropTarget();
                            }
                            ImGui::SameLine();
                            ImGui::BeginGroup();
                            ImGui::Text(texName.c_str());

                            ImGui::PushID(id);
                            if (ImGui::Button("Remove", { 100.0f, 24.0f }))
                            {
                                texuuid = uuid;
                                removeTex = true;
                            }
                            id++;
                            ImGui::PopID();
                            ImGui::EndGroup();
                        }

                        if (removeTex)
                        {
                            if (textures.size() <= 1)
                            {
                                textures[texuuid] = m_DefaultTexture;
                            }
                            else
                                textures.erase(texuuid);
                        }
                        removeTex = false;

                        ImGui::Spacing();
                        ImGui::Spacing();
                        float buttonX = ImGui::GetStyle().WindowPadding.x * 1.75f;
                        ImGui::SetCursorPosX(buttonX);
                        if (ImGui::Button("Add Texture", { 100.0f, 24.0f }))
                        {
                            material->AddTexture(texuuid, m_DefaultTexture);
                        }
                        ImGui::Spacing();
                        ImGui::Spacing();

                        auto& materialParams = material->GetAllVariables();
                        for (auto& param : materialParams)
                        {
                            switch (param->GetType())
                            {
                            case Cgr::ShaderDataType::None:
                                break;
                            case Cgr::ShaderDataType::Float:
                                ImGui::DragFloat(param->GetName().c_str(), static_cast<float*>(param->GetValue()), 0.1f);
                                break;
                            case Cgr::ShaderDataType::Float2:
                                ImGui::DragFloat2(param->GetName().c_str(), static_cast<float*>(param->GetValue()));
                                break;
                            case Cgr::ShaderDataType::Float3:
                                ImGui::ColorEdit3(param->GetName().c_str(), static_cast<float*>(param->GetValue()));
                                break;
                            case Cgr::ShaderDataType::Float4:
                                ImGui::ColorEdit4(param->GetName().c_str(), static_cast<float*>(param->GetValue()));
                                break;
                            case Cgr::ShaderDataType::Mat3:
                                //ImGui::DragFloat2(param->GetName().c_str(), static_cast<float*>(param->GetValue()));
                                break;
                            case Cgr::ShaderDataType::Mat4:
                                //ImGui::DragFloat2(param->GetName().c_str(), static_cast<float*>(param->GetValue()));
                                break;
                            case Cgr::ShaderDataType::Int:
                                ImGui::DragInt(param->GetName().c_str(), static_cast<int*>(param->GetValue()));
                                break;
                            case Cgr::ShaderDataType::Int2:
                                ImGui::DragInt2(param->GetName().c_str(), static_cast<int*>(param->GetValue()));
                                break;
                            case Cgr::ShaderDataType::Int3:
                                ImGui::DragInt3(param->GetName().c_str(), static_cast<int*>(param->GetValue()));
                                break;
                            case Cgr::ShaderDataType::Int4:
                                ImGui::DragInt4(param->GetName().c_str(), static_cast<int*>(param->GetValue()));
                                break;
                            case Cgr::ShaderDataType::Bool:
                                ImGui::Checkbox(param->GetName().c_str(), static_cast<bool*>(param->GetValue()));
                                break;
                            default:
                                break;
                            }
                        }
                        ImGui::TreePop();
                    }
                }
			});
	}
}
