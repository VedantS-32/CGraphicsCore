#pragma once

#include "CGR/Core/Core.h"
#include "ShaderDataTypes.h"

namespace Cgr
{
	static uint32_t s_VertexAttribIndex = 0;

	static int GetElementCount(ShaderDataType type)
	{
		switch (type)
		{
		case Cgr::ShaderDataType::Float:  return 1;		break;
		case Cgr::ShaderDataType::Float2: return 2;		break;
		case Cgr::ShaderDataType::Float3: return 3;		break;
		case Cgr::ShaderDataType::Float4: return 4;		break;
		case Cgr::ShaderDataType::Mat3:   return 3 * 3; break;
		case Cgr::ShaderDataType::Mat4:	  return 4 * 4; break;
		case Cgr::ShaderDataType::Int:    return 1;		break;
		case Cgr::ShaderDataType::Int2:	  return 2;		break;
		case Cgr::ShaderDataType::Int3:	  return 3;		break;
		case Cgr::ShaderDataType::Int4:	  return 4;		break;
		case Cgr::ShaderDataType::Bool:	  return 1;		break;
		default:break;
		}

		CGR_CORE_ASSERT(false, "Unknown data type");

		return 0;
	}

	static int GetShaderDatatypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case Cgr::ShaderDataType::Float:  return 4;			break;
		case Cgr::ShaderDataType::Float2: return 4 * 2;		break;
		case Cgr::ShaderDataType::Float3: return 4 * 3;		break;
		case Cgr::ShaderDataType::Float4: return 4 * 4;		break;
		case Cgr::ShaderDataType::Mat3:   return 4 * 3 * 3; break;
		case Cgr::ShaderDataType::Mat4:	  return 4 * 4 * 4; break;
		case Cgr::ShaderDataType::Int:    return 4;			break;
		case Cgr::ShaderDataType::Int2:	  return 4 * 2;		break;
		case Cgr::ShaderDataType::Int3:	  return 4 * 3;		break;
		case Cgr::ShaderDataType::Int4:	  return 4 * 4;		break;
		case Cgr::ShaderDataType::Bool:	  return 1;			break;
		default:break;
		}

		CGR_CORE_ASSERT(false, "Unknown data type");

		return 0;
	}

	enum class CGR_API BufferDrawUsage
	{
		StaticDraw = 0, StreamDraw, DynamicDraw
	};

	struct CGR_API BufferElement
	{
		std::string Name;
		uint32_t AttribIndex;
		int Count;
		ShaderDataType Type;
		bool IsNormalized;
		int Offset = 0;

		BufferElement(ShaderDataType type, const std::string& name, bool isNormalized = false)
			: AttribIndex(s_VertexAttribIndex), Name(name), Count(GetElementCount(type)),
			  Type(type), IsNormalized(isNormalized)
		{
			s_VertexAttribIndex++;
		}
	};

	class CGR_API BufferLayout
	{
	public:
		BufferLayout() = default;
		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_Elements(elements), m_Stride(0)
		{
			CalculateStrideAndOffset();
		}

		const std::vector<BufferElement>& GetBufferElements() { return m_Elements; }
		int GetStride() const { return m_Stride; }

	public:
		static Scope<BufferLayout> Create(const BufferLayout& elements)
		{
			return CreateScope<BufferLayout>(elements);
		}

	private:
		void CalculateStrideAndOffset()
		{
			int offset = 0;
			for (auto& e : m_Elements)
			{
				e.Offset = offset;
				offset += GetShaderDatatypeSize(e.Type);
				m_Stride += GetShaderDatatypeSize(e.Type);
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		int m_Stride;
	};
	class CGR_API IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {};

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual uint32_t GetCount() = 0;
		virtual void SetData(int64_t offset, uint32_t count, uint32_t* indices) = 0;

	public:
		static Ref<IndexBuffer> Create(uint32_t count, uint32_t* indices, BufferDrawUsage usage = BufferDrawUsage::StaticDraw);
	};

	class CGR_API VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {};

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void SetData(int64_t offset, int64_t size, const void* data) = 0;

	public:
		static Ref<VertexBuffer> Create(int64_t size, const void* data, BufferDrawUsage usage = BufferDrawUsage::StaticDraw);
	};

}
