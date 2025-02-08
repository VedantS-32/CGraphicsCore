#pragma once

#include "CGR/Core/Core.h"
#include "RendererAPI.h"
#include "VertexArray.h"

namespace Cgr
{
	class CGR_API RenderCommand
	{
	public:
		static void Init();

		inline static void SetClearColor(const glm::vec4& color) { s_RendererAPI->SetClearColor(color); }

		inline static void Clear() { s_RendererAPI->Clear(); }

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, count);
		}

		inline static void DrawIndexed(uint32_t count = 0)
		{
			s_RendererAPI->DrawIndexed(count);
		}

		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		// For Client Application
		static RendererAPI& GetRendererAPI();

	private:
		static RendererAPI* s_RendererAPI;
	};
}