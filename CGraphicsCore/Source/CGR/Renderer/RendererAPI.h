#pragma once

#include "CGR/Core/Core.h"
#include <glm/glm.hpp>

namespace Cgr
{
	class VertexArray;
	enum class API
	{
		None = 0, OpenGL
	};

	class CGR_API RendererAPI
	{
	public:
		virtual void Init() = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		virtual void Clear() = 0;
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) = 0;
		virtual void DrawIndexed(uint32_t indexCount) = 0;

		static API GetAPI() { return m_API; }

	protected:
		static API m_API;
	};
}