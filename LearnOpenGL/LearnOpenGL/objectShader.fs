#version 330 core

in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;

out vec4 FragColor;

void main()
{
    float ambientFactor = 0.1f;
    vec3 ambientColor = ambientFactor * lightColor;

    vec3 norm = normalize(Normal);
    // What if we subtracted lightPos from FragPos instead?
    // Also do lightPos and FragPos not need to be normalized individually before subtraction?
    vec3 lightDir = normalize(lightPos - FragPos);
    // Make sure to clamp lower values to 0 (dot products become negative for angles greater than 90 degrees)
    float diffuseFactor = max(dot(norm, lightDir), 0.0f);
    vec3 diffuseColor = diffuseFactor * lightColor;

    vec3 lightImpact = ambientColor + diffuseColor;
    FragColor = vec4(objectColor * lightImpact, 1.0f);
}