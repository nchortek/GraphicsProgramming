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

unsigned int configureTexture(const char* path, const string& directory);

class Model
{
    public:
        Model(const char* path)
        {
            this->loadModel(path);
        }

        void draw(Shader& shader)
        {
            unsigned int meshCount = this->meshes.size();
            for (unsigned int i = 0; i < meshCount; i++)
            {
                this->meshes[i].draw(shader);
            }
        }

        void freeResources()
        {
            unsigned int meshCount = this->meshes.size();
            for (unsigned int i = 0; i < meshCount; i++)
            {
                this->meshes[i].freeResources();
            }
        }

    private:
        // Model data
        vector<Texture> loadedTextures;
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

            this->processNode(scene->mRootNode, scene, 1);
            // cout << "Finished loading model" << endl;
        }

        void processNode(aiNode* node, const aiScene* scene, int count)
        {
            // Process this node's meshes
            unsigned int meshCount = node->mNumMeshes;
            //cout << "Mesh count for node " << count << " is " << meshCount << endl;
            for (unsigned int i = 0; i < meshCount; i++)
            {
                unsigned int meshIndex = node->mMeshes[i];
                aiMesh* mesh = scene->mMeshes[meshIndex];

                Mesh processedMesh = processMesh(mesh, scene);
                meshes.push_back(processedMesh);
            }

            //cout << "Finished processing meshes for node " << count << endl;

            // Recursively process this node's child nodes
            unsigned int childCount = node->mNumChildren;

            //cout << "Child count for node " << count << " is " << childCount << endl;

            for (unsigned int i = 0; i < childCount; i++)
            {
                aiNode* child = node->mChildren[i];
                processNode(child, scene, ++count);
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
            vector<Texture> textures;

            unsigned int textureCount = mat->GetTextureCount(type);
            for (unsigned int i = 0; i < textureCount; i++)
            {
                bool skip = false;
                Texture currentTexture;

                aiString filePath;
                mat->GetTexture(type, i, &filePath);

                unsigned int loadedTexturesCount = this->loadedTextures.size();
                for (unsigned int j = 0; j < loadedTexturesCount; j++)
                {
                    currentTexture = this->loadedTextures[j];
                    if (std::strcmp(currentTexture.path.data(), filePath.C_Str()) == 0)
                    {
                        textures.push_back(currentTexture);

                        skip = true;
                        break;
                    }
                }

                if (!skip)
                {
                    Texture currentTexture;
                    currentTexture.id = configureTexture(filePath.C_Str(), directory);
                    currentTexture.type = typeName;
                    currentTexture.path = filePath.C_Str();
                    textures.push_back(currentTexture);
                    this->loadedTextures.push_back(currentTexture);
                }
            }

            return textures;
        }
};

unsigned int configureTexture(const char* path, const string& directory)
{
    unsigned int texture;
    int width, height, nrChannels;

    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);

    if (data)
    {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // TODO: Again, is this a proper initialization? We need a good default to avoid the unitialized memory issue
        GLenum format = 0;
        if (nrChannels == 1)
        {
            format = GL_RED;
        }
        else if (nrChannels == 3)
        {
            format = GL_RGB;
        }
        else if (nrChannels == 4)
        {
            format = GL_RGBA;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        return texture;
    }
    else
    {
        cout << "Failed to load texture" << endl;

        stbi_image_free(data);
        return 0;
    }
}

#endif