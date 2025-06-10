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
		OpenGLSkybox();
		virtual ~OpenGLSkybox();

		virtual void Bind() override;
		virtual void Bind(uint32_t slot) override;
		virtual void UnBind() override;

		virtual std::vector<Ref<ShaderVariable>>& GetAllVariables() override { return m_ShaderVariables; }
		virtual std::unordered_map<SkyboxSide, AssetHandle>& GetTextureHandles() override { return m_TextureHandles; }
		virtual void AddTextureHandle(const SkyboxSide side, const AssetHandle handle) override { m_TextureHandles[side] = handle; }
		virtual void SetTexture(SkyboxSide side, TextureSpecification spec, const void* data) override;
		virtual void UploadTextures() override;
		virtual void SetShader(Ref<Shader> shader) override;
		virtual Ref<Shader> GetShader() override { return m_Shader; }
		virtual void Render(Camera& camera) override;

	private:
		Ref<Material> m_ENVMapMaterial;
		Ref<Shader> m_Shader;
		std::vector<Ref<ShaderVariable>> m_ShaderVariables;
		std::unordered_map<SkyboxSide, AssetHandle> m_TextureHandles;

	private:
		RendererID m_RendererID;
	};
}
