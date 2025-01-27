#pragma once

#include "CGR/Core/Core.h"
#include "CGR/Asset/Asset.h"

#include <string>

namespace Cgr
{
	enum class ImageFormat
	{
		None = 0,
		R8,
		RGB8,
		RGBA8,
		RGBA16F,
		RGBA32F
	};

	struct TextureSpecification
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		ImageFormat Format = ImageFormat::RGBA8;
		bool GenerateMips = false;
	};

	class CGR_API Texture : public Asset
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual const std::string& GetFilepath() const = 0;
		virtual std::string& GetName() = 0;
		virtual void SetName(const std::string& name) = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void BindActiveTexture(uint32_t slot = 0) const = 0;
		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void Unbind() const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class CGR_API Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<Texture2D> Create(const std::string& path);
		static Ref<Texture2D> Create(const TextureSpecification& spec, const void* data);

		static AssetType GetStaticType() { return AssetType::Texture2D; }
		virtual AssetType GetType() const override { return GetStaticType(); }
	};
}