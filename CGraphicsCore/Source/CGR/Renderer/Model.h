#pragma once

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
	};

	class CGR_API Mesh
	{
	public:
		Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

		Ref<VertexBuffer> GetVertexBuffer() const { return m_VertexBuffer; }
		Ref<IndexBuffer> GetIndexBuffer() const { return m_IndexBuffer; }
		uint32_t GetIndexCount() const { return m_IndicesCount; }

		const std::vector<Vertex>& GetVertices() { return m_Vertices; }
		const std::vector<uint32_t>& GetIndices() { return m_Indices; }
		Ref<Material> GetMaterial() const { return m_Material; }
		void SetMaterial(Ref<Material> material) { m_Material = material; }

		Ref<Shader> m_Shader;
	private:
		Ref<Material> m_Material;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		int32_t m_IndicesCount = 0;
	};

	class CGR_API Model
	{
	public:
		Model(const std::string& modelPath);
		~Model() {}

		const std::string& GetName() { return m_ModelName; }
		glm::mat4& GetModelMatrix() { return m_ModelMatrix; }
		std::vector<Mesh>& GetMeshes() { return m_Meshes; }

		void SetMaterial(uint32_t meshIndex, Ref<Material> material);

		void DrawModel(const Ref<VertexArray> vertexArray, const BufferLayout& layout, Ref<ShaderStorageBuffer> SSBO) const;

	public:
		static Ref<Model> Create(const std::string& modelPath);

	private:
		glm::mat4 m_ModelMatrix{ 1.0f };
		std::string m_ModelPath;
		std::string m_ModelName;
		std::vector<Mesh> m_Meshes;
	};
}
