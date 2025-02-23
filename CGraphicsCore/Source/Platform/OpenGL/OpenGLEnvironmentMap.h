#pragma once

#include "CGR/Renderer/EnvironmentMap.h"
#include "CGR/Renderer/VertexArray.h"
#include "CGR/Renderer/Material.h"

#include "CGR/Core/Reflection.h"

namespace Cgr
{
	class Camera;

	class OpenGLSkybox : public Skybox
	{
	public:
		CLASS(OpenGLSkybox);
		OpenGLSkybox();
		virtual ~OpenGLSkybox();

		virtual void Bind() override;
		virtual void UnBind() override;

		virtual std::vector<Ref<ShaderVariable>>& GetAllVariables() override { return m_ShaderVariables; }
		virtual const std::vector<std::string>& GetTexturePaths() override { return m_TexturePaths; }
		virtual void AddTexturePath(const std::string& path) override { m_TexturePaths.push_back(path); }
		virtual void SetTexture(SkyboxSide side, TextureSpecification spec, const void* data) override;
		virtual void SetShader(Ref<Shader> shader) override;
		virtual Ref<Shader> GetShader() override { return m_Shader; }
		virtual void Render(Camera& camera) override;

	private:
		Ref<Material> m_ENVMapMaterial;
		Ref<Shader> m_Shader;
		std::vector<Ref<ShaderVariable>> m_ShaderVariables;
		std::vector<std::string> m_TexturePaths;

		float m_Rotation = 90.0f;
		float m_Intensity = 1.0f;
		float m_Red = 1.0f;

	private:
		uint32_t m_RendererID;
	};
}
