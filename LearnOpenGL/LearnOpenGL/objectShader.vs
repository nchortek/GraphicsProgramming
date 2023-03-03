#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat3 normalModel;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    vec4 aPosV4 = vec4(aPos, 1.0f);
    gl_Position = projection * view * model * aPosV4;
    FragPos = vec3(model * aPosV4);
    Normal = normalModel * aNormal;
}