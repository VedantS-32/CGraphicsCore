#include "CGRpch.h"
#include "Material.h"

namespace Cgr
{
    Material::Material(Ref<Shader> shader)
        : m_Shader(shader)
    {
        m_UniformBuffer = UniformBuffer::Create("ModelProps");
        m_UniformBuffer->SetBlockBinding(m_Shader->GetRendererID());
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
