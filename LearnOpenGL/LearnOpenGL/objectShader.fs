#version 330 core

in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
    // Ambient Lighting

    float ambientStrength = 0.1f;
    vec3 ambientColor = ambientStrength * lightColor;

    // Diffuse Lighting

    vec3 norm = normalize(Normal);
    // What if we subtracted lightPos from FragPos instead?
    // Also do lightPos and FragPos not need to be normalized individually before subtraction?
    vec3 lightDir = normalize(lightPos - FragPos);
    // Make sure to clamp lower values to 0 (dot products become negative for angles greater than 90 degrees)
    float diffuseFactor = max(dot(norm, lightDir), 0.0f);
    vec3 diffuseColor = diffuseFactor * lightColor;

    // Specular Lighting

    float specularStrength = 0.5f;
    int shininess = 32;
    vec3 viewDir = normalize(viewPos - FragPos);
    // The reflect function expects the first vector to point from the light source towards the fragment's position,
    // but the lightDir vector is currently pointing the other way around: from the fragment towards the light source
    // (this depends on the order of subtraction earlier on when we calculated the lightDir vector). So we just negate
    // lightDir here.
    vec3 reflectDir = reflect(-lightDir, norm);
    float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
    vec3 specularColor = specularStrength * specularFactor * lightColor;

    vec3 lightImpact = ambientColor + diffuseColor + specularColor;
    FragColor = vec4(objectColor * lightImpact, 1.0f);
}