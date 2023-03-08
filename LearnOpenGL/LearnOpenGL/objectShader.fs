#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
    // Ambient Lighting

    vec3 ambientColor = light.ambient * material.ambient;

    // Diffuse Lighting

    vec3 norm = normalize(Normal);
    // What if we subtracted lightPos from FragPos instead?
    // Also do lightPos and FragPos not need to be normalized individually before subtraction?
    vec3 lightDir = normalize(light.position - FragPos);
    // Make sure to clamp lower values to 0 (dot products become negative for angles greater than 90 degrees)
    float diffuseFactor = max(dot(norm, lightDir), 0.0f);
    vec3 diffuseColor = light.diffuse * (diffuseFactor * material.diffuse);

    // Specular Lighting

    vec3 viewDir = normalize(viewPos - FragPos);
    // The reflect function expects the first vector to point from the light source towards the fragment's position,
    // but the lightDir vector is currently pointing the other way around: from the fragment towards the light source
    // (this depends on the order of subtraction earlier on when we calculated the lightDir vector). So we just negate
    // lightDir here.
    vec3 reflectDir = reflect(-lightDir, norm);
    float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
    vec3 specularColor = light.specular * (specularFactor * material.specular);

    vec3 lightAdjustedColor = ambientColor + diffuseColor + specularColor;
    FragColor = vec4(lightAdjustedColor, 1.0f);
}