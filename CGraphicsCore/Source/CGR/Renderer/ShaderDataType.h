#pragma once

#include "CGR/Core/Core.h"
#include <glm/glm.hpp>

namespace Cgr
{
	enum class CGR_API ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

#define SHADERVARIABLE_CLASS_TYPE() virtual const std::string& GetName() override { return m_Name; }\
									virtual ShaderDataType GetType() override { return m_Type; }\
									virtual void* GetValue() override { return static_cast<void*>(&m_Variable); }\
									virtual int64_t GetOffset() override { return m_Offset; }


	//using ShaderVariableTuple = std::tuple<float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4, int, glm::ivec2, glm::ivec3, glm::ivec4, bool>;

	class CGR_API ShaderVariable
	{
	public:
		virtual const std::string& GetName() = 0;
		virtual ShaderDataType GetType() = 0;
		virtual void* GetValue() = 0;
		virtual int64_t GetOffset() = 0;
	};

	class ShaderFloat : public ShaderVariable
	{
	public:
		ShaderFloat(const std::string& name, float variable, int64_t offset)
			: m_Name(name), m_Variable(variable), m_Offset(offset) {}
		SHADERVARIABLE_CLASS_TYPE();

	private:
		ShaderDataType m_Type = ShaderDataType::Float;
		std::string m_Name;
		int64_t m_Offset = 0;
		float m_Variable;
	};

	class ShaderFloat2 : public ShaderVariable
	{
	public:
		ShaderFloat2(const std::string& name, const glm::vec2& variable, int64_t offset)
			: m_Name(name), m_Variable(variable), m_Offset(offset) {}
		SHADERVARIABLE_CLASS_TYPE();

	private:
		ShaderDataType m_Type = ShaderDataType::Float2;
		std::string m_Name;
		int64_t m_Offset = 0;
		glm::vec2 m_Variable;
	};

	class ShaderFloat3 : public ShaderVariable
	{
	public:
		ShaderFloat3(const std::string& name, const glm::vec3& variable, int64_t offset)
			: m_Name(name), m_Variable(variable), m_Offset(offset) {}
		SHADERVARIABLE_CLASS_TYPE();

	private:
		ShaderDataType m_Type = ShaderDataType::Float3;
		std::string m_Name;
		int64_t m_Offset = 0;
		glm::vec3 m_Variable;
	};

	class ShaderFloat4 : public ShaderVariable
	{
	public:
		ShaderFloat4(const std::string& name, const glm::vec4& variable, int64_t offset)
			: m_Name(name), m_Variable(variable), m_Offset(offset) {}
		SHADERVARIABLE_CLASS_TYPE();

	private:
		ShaderDataType m_Type = ShaderDataType::Float4;
		std::string m_Name;
		int64_t m_Offset = 0;
		glm::vec4 m_Variable;
	};

	class ShaderMat3 : public ShaderVariable
	{
	public:
		ShaderMat3(const std::string& name, const glm::mat3& variable, int64_t offset)
			: m_Name(name), m_Variable(variable), m_Offset(offset) {}
		SHADERVARIABLE_CLASS_TYPE();

	private:
		ShaderDataType m_Type = ShaderDataType::Mat3;
		std::string m_Name;
		int64_t m_Offset = 0;
		glm::mat3 m_Variable;
	};

	class ShaderMat4 : public ShaderVariable
	{
	public:
		ShaderMat4(const std::string& name, const glm::mat4& variable, int64_t offset)
			: m_Name(name), m_Variable(variable), m_Offset(offset) {}
		SHADERVARIABLE_CLASS_TYPE();

	private:
		ShaderDataType m_Type = ShaderDataType::Mat4;
		std::string m_Name;
		int64_t m_Offset = 0;
		glm::mat4 m_Variable;
	};

	class ShaderInt : public ShaderVariable
	{
	public:
		ShaderInt(const std::string& name, int variable, int64_t offset)
			: m_Name(name), m_Variable(variable), m_Offset(offset) {}
		SHADERVARIABLE_CLASS_TYPE();

	private:
		ShaderDataType m_Type = ShaderDataType::Int;
		std::string m_Name;
		int64_t m_Offset = 0;
		int m_Variable;
	};

	class ShaderInt2 : public ShaderVariable
	{
	public:
		ShaderInt2(const std::string& name, const glm::ivec2& variable, int64_t offset)
			: m_Name(name), m_Variable(variable), m_Offset(offset) {}
		SHADERVARIABLE_CLASS_TYPE();

	private:
		ShaderDataType m_Type = ShaderDataType::Int2;
		std::string m_Name;
		int64_t m_Offset = 0;
		glm::ivec2 m_Variable;
	};

	class ShaderInt3 : public ShaderVariable
	{
	public:
		ShaderInt3(const std::string& name, const glm::ivec3& variable, int64_t offset)
			: m_Name(name), m_Variable(variable), m_Offset(offset) {}
		SHADERVARIABLE_CLASS_TYPE();

	private:
		ShaderDataType m_Type = ShaderDataType::Int3;
		std::string m_Name;
		int64_t m_Offset = 0;
		glm::ivec3 m_Variable;
	};

	class ShaderInt4 : public ShaderVariable
	{
	public:
		ShaderInt4(const std::string& name, const glm::ivec4& variable, int64_t offset)
			: m_Name(name), m_Variable(variable), m_Offset(offset) {}
		SHADERVARIABLE_CLASS_TYPE();

	private:
		ShaderDataType m_Type = ShaderDataType::Int4;
		std::string m_Name;
		int64_t m_Offset = 0;
		glm::ivec4 m_Variable;
	};

	class ShaderBool : public ShaderVariable
	{
	public:
		ShaderBool(const std::string& name, bool variable, int64_t offset)
			: m_Name(name), m_Variable(variable), m_Offset(offset) {}
		SHADERVARIABLE_CLASS_TYPE();

	private:
		ShaderDataType m_Type = ShaderDataType::Bool;
		std::string m_Name;
		int64_t m_Offset = 0;
		bool m_Variable;
	};

}