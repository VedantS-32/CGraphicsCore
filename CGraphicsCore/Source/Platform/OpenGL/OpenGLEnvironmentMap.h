#pragma once

#include "CGR/Renderer/EnvironmentMap.h"
#include "CGR/Renderer/VertexArray.h"
#include "CGR/Renderer/Material.h"

namespace Cgr
{
	class Camera;

	class OpenGLCubeMap : public CubeMap
	{
	public:
		OpenGLCubeMap();
		virtual ~OpenGLCubeMap();

		virtual void Bind() override;
		virtual void UnBind() override;

		virtual void Render(Camera& camera) override;

	private:
		Ref<VertexArray> m_ENVMapVertexArray;
		Ref<VertexBuffer> m_ENVMapVertexBuffer;
		Ref<Material> m_ENVMapMaterial;
		Ref<Shader> m_Shader;

	private:
		uint32_t m_RendererID;
	};
}
