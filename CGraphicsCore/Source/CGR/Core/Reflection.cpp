#include "CGRpch.h"
#include "Reflection.h"

namespace Cgr
{
	void Reflection::ReflectClass(const std::string& className)
	{
		if(!m_AttributeClassMap.contains(className))
		{
			CGR_CORE_ASSERT(false, std::format("{} doesn't exists in reflection system", className));
			return;
		}

		int id = 0;
		for (auto& attribute : *m_AttributeClassMap[className])
		{
			ImGui::PushID(id++);
			if (attribute->Type == VariableType::RendererID)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
				uint32_t textureID = *static_cast<uint32_t*>(attribute->Value);
				ImGui::ImageButton(reinterpret_cast<void*>(static_cast<uintptr_t>(textureID)), ImVec2(98, 98), { 0, 1 }, { 1, 0 });
				ImGui::PopStyleColor();
			}
			else if (attribute->Type == VariableType::Float)
			{
				if (ImGui::DragFloat(attribute->Name.c_str(), static_cast<float*>(attribute->Value), 0.1f))
					m_ClassUpdateFunctions[className]();
			}
			else if (attribute->Type == VariableType::Double)
			{
				if (ImGui::DragScalar(attribute->Name.c_str(), ImGuiDataType_Double, static_cast<double*>(attribute->Value), 0.1f))
					m_ClassUpdateFunctions[className]();
			}
			else if (attribute->Type == VariableType::Float2)
			{
				if (ImGui::DragFloat2(attribute->Name.c_str(), static_cast<float*>(attribute->Value), 0.1f))
					m_ClassUpdateFunctions[className]();
			}
			else if (attribute->Type == VariableType::Float3)
			{
				if (ImGui::DragFloat3(attribute->Name.c_str(), static_cast<float*>(attribute->Value), 0.1f))
					m_ClassUpdateFunctions[className]();
			}
			else if (attribute->Type == VariableType::Float4)
			{
				if (ImGui::DragFloat4(attribute->Name.c_str(), static_cast<float*>(attribute->Value), 0.1f))
					m_ClassUpdateFunctions[className]();
			}
			else if (attribute->Type == VariableType::Int)
			{
				if (ImGui::DragInt(attribute->Name.c_str(), static_cast<int*>(attribute->Value), 1))
					m_ClassUpdateFunctions[className]();
			}
			else if (attribute->Type == VariableType::Int2)
			{
				if (ImGui::DragInt2(attribute->Name.c_str(), static_cast<int*>(attribute->Value), 1))
					m_ClassUpdateFunctions[className]();
			}
			else if (attribute->Type == VariableType::Int3)
			{
				if (ImGui::DragInt3(attribute->Name.c_str(), static_cast<int*>(attribute->Value), 1))
					m_ClassUpdateFunctions[className]();
			}
			else if (attribute->Type == VariableType::Int4)
			{
				if (ImGui::DragInt4(attribute->Name.c_str(), static_cast<int*>(attribute->Value), 1))
					m_ClassUpdateFunctions[className]();
			}
			else if (attribute->Type == VariableType::UInt)
			{
				if (ImGui::DragScalar(attribute->Name.c_str(), ImGuiDataType_U32, static_cast<uint32_t*>(attribute->Value), 1))
					m_ClassUpdateFunctions[className]();
			}
			ImGui::PopID();
		}
	}
}
