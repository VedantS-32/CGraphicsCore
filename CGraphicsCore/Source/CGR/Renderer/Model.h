#pragma once

#include "CGR/Asset/Asset.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "CGR/Core/Core.h"
#include "Material.h"

#include <glm/glm.hpp>

namespace Cgr
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;
		glm::vec3 Tangent;
	};

	class CGR_API Mesh
	{
	public:
		Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, uint32_t materialIndex);

		Ref<VertexBuffer> GetVertexBuffer() const { return m_VertexBuffer; }
		Ref<IndexBuffer> GetIndexBuffer() const { return m_IndexBuffer; }
		uint32_t GetIndexCount() const { return m_IndicesCount; }

		const std::vector<Vertex>& GetVertices() { return m_Vertices; }
		const std::vector<uint32_t>& GetIndices() { return m_Indices; }
		uint32_t GetMaterialIndex() const { return m_MaterialIndex; }

		Ref<Shader> m_Shader;
	private:
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		uint32_t m_MaterialIndex = 0;
		uint32_t m_IndicesCount = 0;
	};

	class CGR_API Model : public Asset
	{
	public:
		Model() = default;
		Model(const std::string& modelPath);
		~Model() {}

		const std::string& GetName() { return m_ModelName; }
		glm::mat4& GetModelMatrix() { return m_ModelMatrix; }
		std::vector<Mesh>& GetMeshes() { return m_Meshes; }

		void AddMaterial(Ref<Material> material);
		void AddMaterial(Ref<Shader> shader);
		void SetMaterial(uint32_t materialIndex, Ref<Material> material);
		Ref<Material> GetMaterial(uint32_t materialIndex) { return m_Materials.at(materialIndex); }

		void DrawModel(const Ref<VertexArray> vertexArray, const BufferLayout& layout, Ref<ShaderStorageBuffer> SSBO);

		static AssetType GetStaticType() { return AssetType::Model; }
		virtual AssetType GetType() const { return GetStaticType(); }

	public:
		static Ref<Model> Create(const std::string& modelPath);
		static Ref<Model> Create();

	private:
		glm::mat4 m_ModelMatrix{ 1.0f };
		std::string m_ModelPath;
		std::string m_ModelName;
		std::vector<Mesh> m_Meshes;
		std::vector<Ref<Material>> m_Materials;
	};
}
