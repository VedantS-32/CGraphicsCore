#include "CGRpch.h"
#include "ModelImporter.h"

#include "CGR/Core/Application.h"
#include "CGR/Asset/AssetManager.h"

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

		uint32_t indicesCount = mesh->mNumFaces;
		indices.reserve(indicesCount * sizeof(uint32_t));

		uint32_t materialIndex = mesh->mMaterialIndex;

		CGR_CORE_INFO("Name: {0}, Material index: {1}", mesh->mName.C_Str(), mesh->mMaterialIndex);

		for (uint32_t i = 0; i < verticesCount; i++)
		{
			Vertex vertex;
			// process vertex positions, normals and texture coordinates
			glm::vec3 vector;
			vector.x = static_cast<float>(mesh->mVertices[i].x);
			vector.y = static_cast<float>(mesh->mVertices[i].y);
			vector.z = static_cast<float>(mesh->mVertices[i].z);
			vertex.Position = vector;

			vector.x = static_cast<float>(mesh->mNormals[i].x);
			vector.y = static_cast<float>(mesh->mNormals[i].y);
			vector.z = static_cast<float>(mesh->mNormals[i].z);
			vertex.Normal = vector;

			vector.x = static_cast<float>(mesh->mTangents[i].x);
			vector.y = static_cast<float>(mesh->mTangents[i].y);
			vector.z = static_cast<float>(mesh->mTangents[i].z);
			vertex.Tangent = vector;

			if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				glm::vec2 vec;
				vec.x = static_cast<float>(mesh->mTextureCoords[0][i].x);
				vec.y = static_cast<float>(mesh->mTextureCoords[0][i].y);
				vertex.TexCoord = vec;
			}
			else
				vertex.TexCoord = glm::vec2(0.0f, 0.0f);


			vertices.push_back(vertex);
		}
		// process indices
		for (uint32_t i = 0; i < indicesCount; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		return Mesh(vertices, indices, materialIndex);
	}

	static void LoadMaterialTextures(Ref<Material> material, aiMaterial* mat, aiTextureType type, const std::string& name)
	{
		std::vector<Ref<Texture2D>> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			AssetHandle handle;
			aiString str;
			mat->GetTexture(type, i, &str);

			auto assetManager = Application::Get().GetAssetManager();
			handle = assetManager->ImportAsset(str.C_Str());

			auto texture = assetManager->GetAsset<Texture2D>(handle);
			material->AddTexture(std::format("{}{}", name, i), texture);
			textures.emplace_back(texture);
		}
	}

	// For importing files with .fbx, .obj and others
	static void processNode(bool isFirst, Ref<Model> model, const aiNode* node, const aiScene* scene)
	{
		// process all the node's meshes (if any)
		auto& meshes = model->GetMeshes();
		uint32_t meshesCount = node->mNumMeshes;

		for (uint32_t i = 0; i < meshesCount; i++)
		{
			int currentMatIdx = -1;
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			uint32_t matIdx = mesh->mMaterialIndex;
			if (currentMatIdx != matIdx)
			{
				currentMatIdx = matIdx;
				aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];

				auto assetManager = Application::Get().GetAssetManager();
				auto defaultMaterialHandle = assetManager->GetDefaultAssetHandle(AssetType::Material);
				Ref<Material> material = assetManager->GetAsset<Material>(defaultMaterialHandle);

				LoadMaterialTextures(material, aiMat, aiTextureType_DIFFUSE, "Diffuse");
				LoadMaterialTextures(material, aiMat, aiTextureType_NORMALS, "Normal");
				model->AddMaterial(material);
			}
			meshes.push_back(processMesh(mesh, scene));
		}
		// then do the same for each of its children
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			processNode(true, model, node->mChildren[i], scene);
		}
	}

	// For importing from .csmesh
	static void processNode(Ref<Model> model, const aiNode* node, const aiScene* scene)
	{
		// process all the node's meshes (if any)
		auto& meshes = model->GetMeshes();
		uint32_t meshesCount = node->mNumMeshes;

		for (uint32_t i = 0; i < meshesCount; i++)
		{
			int currentMatIdx = -1;
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			uint32_t matIdx = mesh->mMaterialIndex;
			if (currentMatIdx != matIdx)
			{
				currentMatIdx = matIdx;
				aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];
				auto assetManager = Application::Get().GetAssetManager();
				auto defaultMaterialHandle = assetManager->GetDefaultAssetHandle(AssetType::Material);
				Ref<Material> material = assetManager->GetAsset<Material>(defaultMaterialHandle);

				model->AddMaterial(material);
			}
			meshes.push_back(processMesh(mesh, scene));
		}
		// then do the same for each of its children
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			processNode(model, node->mChildren[i], scene);
		}
	}
    
	Ref<Model> ModelImporter::ImportModel(AssetHandle handle, const AssetMetadata& metadata)
    {
        return LoadModel(metadata.Path);
    }

    Ref<Model> ModelImporter::LoadModel(const std::filesystem::path& filePath)
    {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filePath.string(),
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
			return nullptr;
		}
		auto model = Model::Create();
		if (filePath.extension() == ".csmesh")
			processNode(model, scene->mRootNode, scene);
		else
			processNode(true, model, scene->mRootNode, scene);

		CGR_CORE_INFO("Imported Model asset, path: {0}", filePath.string());

		return model;
    }
}
