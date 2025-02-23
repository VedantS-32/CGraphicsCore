#include "CGRpch.h"
#include "Reflection.h"

#include "Application.h"

namespace Cgr
{
	void Reflection::ReflectClass(const std::string& className)
	{
		bool isChanged = false;
		for (auto& attribute : *m_AttributeClassMap[className])
		{
			if (ImGui::DragFloat(attribute->Name.c_str(), static_cast<float*>(attribute->Value), 0.1f))
				m_ClassUpdateFunctions[className]();
		}
	}
}
