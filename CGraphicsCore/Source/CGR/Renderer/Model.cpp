#include "CGRpch.h"
#include "Model.h"

#include "RenderCommand.h"
#include <glad/glad.h>

#define ASSIMP_DLL
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Cgr
{
	static Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		uint32_t verticesCount = mesh->mNumVertices;
		vertices.reserve(verticesCount * sizeof(Vertex));

		uint32_t m_IndicesCount = mesh->mNumFaces;
		indices.reserve(m_IndicesCount * sizeof(uint32_t));

		for (unsigned int i = 0; i < verticesCount; i++)
		{
			Vertex vertex;
			// process vertex positions, normals and texture coordinates
			glm::vec3 vector;
			vector.x = (float)mesh->mVertices[i].x;
			vector.y = (float)mesh->mVertices[i].y;
			vector.z = (float)mesh->mVertices[i].z;
			vertex.Position = vector;

			vector.x = (float)mesh->mNormals[i].x;
			vector.y = (float)mesh->mNormals[i].y;
			vector.z = (float)mesh->mNormals[i].z;
			vertex.Normal = vector;

			if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				glm::vec2 vec;
				vec.x = (float)mesh->mTextureCoords[0][i].x;
				vec.y = (float)mesh->mTextureCoords[0][i].y;
				vertex.TexCoord = vec;
			}
			else
				vertex.TexCoord = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}
		// process indices
		for (uint32_t i = 0; i < m_IndicesCount; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		return Mesh(vertices, indices);
	}

	static void processNode(Model& model, const aiNode* node, const aiScene* scene)
	{
		// process all the node's meshes (if any)
		auto& meshes = model.GetMeshes();
		uint32_t meshesCount = node->mNumMeshes;

		for (uint32_t i = 0; i < meshesCount; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		// then do the same for each of its children
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			processNode(model, node->mChildren[i], scene);
		}
	}

	static int loadMeshAsset(Model& model, const std::string& modelPath)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(modelPath,
			aiProcess_Triangulate |          // Essential: Convert all geometries to triangles
			aiProcess_GenNormals |          // Generate smooth normals if missing
			aiProcess_CalcTangentSpace |    // Essential for normal mapping
			aiProcess_ImproveCacheLocality | // Important for rendering performance
			aiProcess_FindInvalidData |     // Remove invalid/corrupt data
			aiProcess_OptimizeMeshes |      // Join small meshes, good for performance
			aiProcess_ValidateDataStructure // Ensure data integrity
		);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			CGR_CORE_ERROR("ERROR::ASSIMP:: {0}", importer.GetErrorString());
			return -1;
		}

		processNode(model, scene->mRootNode, scene);

		return 0;
	}

	Model::Model(const std::string& modelPath)
		: m_ModelPath(modelPath)
	{
		auto lastSlash = modelPath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = modelPath.rfind('.');
		auto count = lastDot == std::string::npos ? modelPath.size() - lastSlash : lastDot - lastSlash;
		m_ModelName = modelPath.substr(lastSlash, count);

		loadMeshAsset(*this, modelPath);
	}

	void Model::SetMaterial(uint32_t meshIndex, Ref<Material> material)
	{
		m_Meshes[meshIndex].SetMaterial(material);
	}

	void Model::DrawModel(const Ref<VertexArray> vertexArray, const BufferLayout& layout, Ref<ShaderStorageBuffer> SSBO) const
	{
		for (auto& mesh : m_Meshes)
		{
			mesh.GetVertexBuffer()->Bind();
			mesh.GetIndexBuffer()->Bind();
			vertexArray->SetBufferLayout(layout);
			mesh.GetMaterial()->UpdateSSBOParameters(SSBO);

			glDrawElements(GL_TRIANGLES, mesh.GetIndexCount(), GL_UNSIGNED_INT, nullptr);
		}
	}

	Ref<Model> Model::Create(const std::string& modelPath)
	{
		return CreateRef<Model>(modelPath);
	}

	Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
		: m_Vertices(vertices), m_Indices(indices), m_IndicesCount(static_cast<uint32_t>(indices.size()))
	{
		m_VertexBuffer = VertexBuffer::Create(m_Vertices.size() * sizeof(Vertex), m_Vertices.data());
		m_IndexBuffer = IndexBuffer::Create(m_IndicesCount, m_Indices.data());
		m_Material = Material::Create(Shader::Create("Content/Shader/Cube.glsl"));
	}
}
