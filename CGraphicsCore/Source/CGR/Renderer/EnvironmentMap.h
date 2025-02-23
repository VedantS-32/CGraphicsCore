#pragma once

#include "CGR/Core/Core.h"
#include "CGR/Asset/Asset.h"
#include "CGR/Renderer/ShaderDatatype.h"

namespace Cgr
{
	class Camera;
	class Shader;
	enum class ImageFormat;
	struct TextureSpecification;

	enum class SkyboxSide
	{
		PositiveX,
		NegativeX,
		NegativeY,
		PositiveY,
		PositiveZ,
		NegativeZ,
	};

	class CGR_API Skybox : public Asset
	{
	public:
		virtual ~Skybox() = default;

		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		virtual std::vector<Ref<ShaderVariable>>& GetAllVariables() = 0;
		virtual const std::vector<std::string>& GetTexturePaths() = 0;
		virtual void AddTexturePath(const std::string& path) = 0;
		virtual void SetTexture(SkyboxSide side, TextureSpecification spec, const void* data) = 0;
		virtual void SetShader(Ref<Shader> shader) = 0;
		virtual Ref<Shader> GetShader() = 0;
		virtual void Render(Camera& camera) = 0;

	public:
		static AssetType GetStaticType() { return AssetType::Skybox; }
		virtual AssetType GetType() const { return GetStaticType(); }

	public:
		static Ref<Skybox> Create();
	};
}