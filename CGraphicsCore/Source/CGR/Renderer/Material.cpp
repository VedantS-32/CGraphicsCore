#include "CGRpch.h"
#include "Material.h"

namespace Cgr
{
    Material::Material(Ref<Shader> shader)
        : m_Shader(shader), m_Name(shader->GetName())
    {
        m_Shader->ExtractSSBOParameters(this);
        AddTexture("Diffuse", Texture2D::Create("Content/Texture/MossyCobble/MossyCobbleDiffuse.jpg"));
        AddTexture("Normal", Texture2D::Create("Content/Texture/MossyCobble/MossyCobbleNormal.jpg"));
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
}
