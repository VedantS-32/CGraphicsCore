#pragma once

#include <unordered_map>
#include <any>
#include <functional>

#ifndef _MSC_VER
	#include <cxxabi.h>
	#define CGR_NEEDS_DEMANGLE
#endif

namespace Cgr
{
	enum class CGR_API VariableType
	{
		None = 0, Float, Float2, Float3, Float4, Double, Mat3, Mat4, Int, Int2, Int3, Int4, Bool, Char, String, CharPtr, ConstCharPtr
	};

	struct CGR_API Attribute
	{
		VariableType Type = VariableType::None;
		std::string ClassName = "Class";
		std::string Name = "Default Name";
		void* Value = nullptr;
	};

	// Vector of reflected attributes of class
	using Attributes = std::vector<Ref<Attribute>>;
	using AttributeClassMap = std::unordered_map<std::string, Attributes*>;

	class CGR_API Reflection
	{
	public:
		// Always call between ImGui::Begin() and ImGui::End()
		void ReflectClass(const std::string& className);

		void AddAttribute(const std::string& className, Attributes* attributeMap)
		{
			m_AttributeClassMap[className] = attributeMap;
		}

		void AddClassUpdateFunction(const std::string& className, std::function<void()> updateFunction)
		{
			m_ClassUpdateFunctions[className] = updateFunction;
		}

		Attributes* GetAttribute(const std::string& name)
		{
			return m_AttributeClassMap[name.c_str()];
		}

	private:
		AttributeClassMap m_AttributeClassMap;
		std::unordered_map<std::string, std::function<void()>> m_ClassUpdateFunctions;
	};

	namespace Utils
	{
		// Demangle type names (Only needed for GCC/Clang)
		static std::string DemangleTypeName(const char* name)
		{
		#ifdef CGR_NEEDS_DEMANGLE
			std::cout << name << std::endl;
			int status;
			char* demangled = abi::__cxa_demangle(name, 0, 0, &status);
			std::string result(demangled ? demangled : name);
			free(demangled);
			std::cout << result << std::endl;
			return result;
		#else
			return name;  // MSVC doesn't need demangling
		#endif
		}

		static VariableType GetTypeFromAttribute(const std::any& variable)
		{
			std::string typeName = DemangleTypeName(variable.type().name());

			if (typeName == "float") return VariableType::Float;
			if (typeName == "int") return VariableType::Int;
			if (typeName == "bool") return VariableType::Bool;
			if (typeName == "std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >")
				return VariableType::String;
			if (typeName == "double") return VariableType::Double;
			if (typeName == "glm::vec2") return VariableType::Float2;
			if (typeName == "glm::vec3") return VariableType::Float3;
			if (typeName == "glm::vec4") return VariableType::Float4;
			if (typeName == "glm::mat3") return VariableType::Mat3;
			if (typeName == "glm::mat4") return VariableType::Mat4;
			if (typeName == "glm::ivec2") return VariableType::Int2;
			if (typeName == "glm::ivec3") return VariableType::Int3;
			if (typeName == "glm::ivec4") return VariableType::Int4;
			if (typeName == "char") return VariableType::Char;
			if (typeName == "char*") return VariableType::CharPtr;
			if (typeName == "char const*") return VariableType::CharPtr;

			return VariableType::None;  // Unknown type
		}
	}

#define CLASS(className)\
	const char* r_ClassName = #className;\
	Attributes r_Attributes;\
	void OnAttributeChange();

#define REFLECT()\
	auto* reflectionSystem = Application::Get().GetReflectionSystem();\
	reflectionSystem->AddAttribute(r_ClassName, &r_Attributes);\
	reflectionSystem->AddClassUpdateFunction(r_ClassName, CGR_BIND_EVENT_FN(OnAttributeChange))

#define ATTRIBUTE(name, variable) \
    r_Attributes.emplace_back(CreateRef<Attribute>(Utils::GetTypeFromAttribute(variable), r_ClassName, #name, &variable));
}