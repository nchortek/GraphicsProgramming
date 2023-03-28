#ifndef MODEL_H
#define MODEL_H

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <fstream>
#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <map>
#include <ModelLoading/mesh.h>
#include <Shaders/shader.h>
#include <string>
#include <sstream>
#include <Textures/stb_image.h>
#include <vector>

using namespace std;

class Model
{
    public:
        Model(char* path)
        {
            this->loadModel(path);
        }

        void Draw(Shader& shader)
        {
            unsigned int meshCount = this->meshes.size();
            for (unsigned int i = 0; i < meshCount; i++)
            {
                this->meshes[i].Draw(shader);
            }
        }

    private:
        // Model data
        vector<Mesh> meshes;
        string directory;

        void loadModel(string path)
        {
            Assimp::Importer import;
            const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

            if (!scene
                || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE
                || !scene->mRootNode)
            {
                cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
                return;
            }

            this->directory = path.substr(0, path.find_last_of('/'));

            this->processNode(scene->mRootNode, scene);
        }

        void processNode(aiNode* node, const aiScene* scene)
        {
            // Process this node's meshes
            unsigned int meshCount = node->mNumMeshes;
            for (unsigned int i = 0; i < meshCount; i++)
            {
                unsigned int meshIndex = node->mMeshes[i];
                aiMesh* mesh = scene->mMeshes[meshIndex];

                Mesh processedMesh = processMesh(mesh, scene);
                meshes.push_back(processedMesh);
            }

            // Recursively process this node's child nodes
            unsigned int childCount = node->mNumChildren;
            for (unsigned int i = 0; i < childCount; i++)
            {
                aiNode* child = node->mChildren[i];
                processNode(child, scene);
            }
        }

        Mesh processMesh(aiMesh* mesh, const aiScene* scene)
        {
            vector<Vertex> vertices;
            vector<unsigned int> indices;
            vector<Texture> textures;

            unsigned int numVertices = mesh->mNumVertices;
            for (unsigned int i = 0; i < numVertices; i++)
            {
                Vertex vertex;

                // Process vertex coordinates
                aiVector3D meshVertex = mesh->mVertices[i];
                vertex.position = glm::vec3(meshVertex.x, meshVertex.y, meshVertex.z);

                // Process normals
                aiVector3D meshNormal = mesh->mNormals[i];
                vertex.normal = glm::vec3(meshNormal.x, meshNormal.y, meshNormal.z);

                // Process texture coordinates
                // 
                // mTextureCoords will contain a null pointer if there is no texture
                // associated with the given index.
                // 
                // Also note that we can safely index into the [0] position here because if numVertices is less than 1
                // we would not make it inside this for loop.
                if (mesh->mTextureCoords[0])
                {
                    aiVector3D meshTextureCoord = mesh->mTextureCoords[0][i];
                    vertex.texCoords = glm::vec2(meshTextureCoord.x, meshTextureCoord.y);
                }
                else
                {
                    // Question: Is this an okay default? If the vertex doesn't have texture coordinates should we store it as null?
                    vertex.texCoords = glm::vec2(0.0f, 0.0f);
                }

                vertices.push_back(vertex);
            }

            // Process indices
            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];

                unsigned int numIndices = face.mNumIndices;
                for (unsigned int j = 0; j < numIndices; j++)
                {
                    unsigned int index = face.mIndices[j];
                    indices.push_back(index);
                }
            }

            // Process material
            // 
            // Question: mMaterialIndex is an unsigned int, so is the following check necessary?
            // Doesn't it have to be 0 or larger?
            unsigned int meshMaterialIndex = mesh->mMaterialIndex;
            if (meshMaterialIndex >= 0)
            {
                aiMaterial* material = scene->mMaterials[meshMaterialIndex];

                // TODO: We really should store the "texture_diffuse" and "texture_specular" strings as global constants somewhere
                // so they only have to be updated in a single location
                vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
                textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

                vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
                textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            }

            return Mesh(vertices, indices, textures);
        }

        vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
        {

        }
};

#endif