#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Shaders/shader.h>
#include <string>
#include <vector>

using namespace std;

struct Vertex
{
    // Model space position coordinate
    glm::vec3 position;

    // Model space normal vector
    glm::vec3 normal;

    // Texture coordinates
    glm::vec2 texCoords;
};

struct Texture
{
    // Texture id
    unsigned int id;

    // The name of a texture, in the format of
    // "texture_diffuseN" or "texture_specularN", where
    // N is a number ranging from 1 to the maximum
    // number of textures allowed by OpenGL
    string type;
};

class Mesh
{
    public:
        // Mesh data
        vector<Vertex>       vertices;
        vector<unsigned int> indices;
        vector<Texture>      textures;

        // Vertex Array Object
        unsigned int VAO;

        // Mesh constructor
        Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
        {
            this->vertices = vertices;
            this->indices = indices;
            this->textures = textures;
            this->setupMesh();
        }

        // Renders the mesh
        void Draw(Shader& shader)
        {
            unsigned int diffuseIndex = 1,
                specularIndex = 1;

            for (unsigned int i = 0; i < this->textures.size(); i++)
            {
                Texture currentTexture = this->textures[i];

                // GL_TEXTUREN are just sequential ints which is why we can 
                // just add the current index here
                glActiveTexture(GL_TEXTURE0 + i);

                int typedTextureIndex = 0;
                string name = currentTexture.type;

                // TODO: Might be better off using an enum and switch case here as opposed to hard coding the string
                if (name == "texture_diffuse")
                {
                    typedTextureIndex = diffuseIndex;
                    diffuseIndex++;
                }
                else if (name == "texture_specular")
                {
                    typedTextureIndex = specularIndex;
                    specularIndex++;
                }

                string uniform = "material." + name + to_string(typedTextureIndex);
                shader.setInt(uniform.c_str(), i);

                glBindTexture(GL_TEXTURE_2D, currentTexture.id);
            }

            // Best practice to reset to defaults
            glActiveTexture(GL_TEXTURE0);

            // Render
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);

            // Unbind VAO
            glBindVertexArray(0);
        }

    private:
        // Render data (Vertex Buffer Object and Element Buffer Object)
        unsigned int VBO, EBO;

        // Initializes our VAO, VBO, and EBO
        void setupMesh()
        {
            glGenVertexArrays(1, &(this->VAO));
            glGenBuffers(1, &(this->VBO));
            glGenBuffers(1, &(this->EBO));

            glBindVertexArray(this->VAO);

            glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices), &(this->vertices[0]), GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(this->indices), &(this->indices[0]), GL_STATIC_DRAW);

            // Set up Vertex position attribute
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

            // Set up Vertex normal attribute
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

            // Set up Vertex texture coordinates attribute
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

            // Unbind VAO
            glBindVertexArray(0);
        }
};

#endif