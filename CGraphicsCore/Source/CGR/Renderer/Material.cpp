#include "CGRpch.h"
#include "Material.h"

#include "CGR/Core/Application.h"

namespace Cgr
{
    Material::Material()
    {
        auto assetManager = Application::Get().GetAssetManager();
        auto defaultShaderHandle = assetManager->GetDefaultAssetHandle(AssetType::Shader);
        m_Shader = assetManager->GetAsset<Shader>(defaultShaderHandle);
        m_Shader->ExtractSSBOParameters(this);
        m_Textures.reserve(6);
    }

    Material::Material(Ref<Shader> shader)
        : m_Shader(shader), m_Name(shader->GetName())
    {
        m_Shader->ExtractSSBOParameters(this);
    }

    void Material::SetShader(Ref<Shader> shader)
    {
        m_Shader = shader;
        m_Shader->ExtractSSBOParameters(this);
    }

    void Material::UpdateSSBOParameters(Ref<ShaderStorageBuffer> SSBO)
    {
        m_Shader->UpdateSSBOParameters(this, SSBO);
    }

    Ref<Material> Material::Create(Ref<Shader> shader)
    {
        return CreateRef<Material>(shader);
    }
    Ref<Material> Material::Create()
    {
        return CreateRef<Material>();
    }
}
