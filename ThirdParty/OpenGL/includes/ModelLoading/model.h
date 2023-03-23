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

        }

    private:
        // Model data
        vector<Mesh> meshes;
        string directory;

        void loadModel(string path)
        {

        }

        void processNode(aiNode* node, const aiScene* scene)
        {

        }

        Mesh processMesh(aiMesh* mesh, const aiScene* scene)
        {

        }

        vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
        {

        }
};

#endif