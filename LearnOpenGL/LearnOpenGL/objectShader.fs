#version 330 core

struct Material {
    sampler2D diffuseMap;
    sampler2D specularMap;
    float shininess;
};

struct Light {
    vec4 lightVector;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
    vec3 diffuseTexture = texture(material.diffuseMap, TexCoords).rgb;

    // Ambient Lighting

    vec3 ambientColor = light.ambient * diffuseTexture;

    // Diffuse Lighting

    vec3 norm = normalize(Normal);

    vec3 lightDir;
    // Light Vector is a positon vector
    if (light.lightVector.w != 0.0f) {
        lightDir = normalize(light.lightVector.xyz - FragPos);
    } else {
        // Light Vector is a direction vector
        // Note that we negate the light vector so we have it points towards the light source
        lightDir = normalize(-light.lightVector.xyz);
    }

    // Make sure to clamp lower values to 0 (dot products become negative for angles greater than 90 degrees)
    float diffuseFactor = max(dot(norm, lightDir), 0.0f);
    vec3 diffuseColor = light.diffuse * diffuseFactor * diffuseTexture;

    // Specular Lighting

    vec3 viewDir = normalize(viewPos - FragPos);
    // The reflect function expects the first vector to point from the light source towards the fragment's position,
    // but the lightDir vector is currently pointing the other way around: from the fragment towards the light source
    // (this depends on the order of subtraction earlier on when we calculated the lightDir vector). So we just negate
    // lightDir here.
    vec3 reflectDir = reflect(-lightDir, norm);
    float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
    vec3 specularColor = light.specular * specularFactor * texture(material.specularMap, TexCoords).rgb;

    vec3 lightAdjustedColor = ambientColor + diffuseColor + specularColor;
    FragColor = vec4(lightAdjustedColor, 1.0f);
}