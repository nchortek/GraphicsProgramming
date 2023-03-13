#version 330 core

struct Material {
    sampler2D diffuseMap;
    sampler2D specularMap;
    float shininess;
};

struct Light {
    // Spotlight Properties
    vec3 position;
    vec3 direction;
    float cutOff;

    // Color Properties
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    // Attentuation Properties
    float constant;
    float linear;
    float quadratic;
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
    // We use ambient lighting for objects both inside and outside the spotlight so calculate it first
    vec3 diffuseTexture = texture(material.diffuseMap, TexCoords).rgb;
    vec3 ambientColor = light.ambient * diffuseTexture;

    vec3 lightDir = normalize(light.position - FragPos);
    float theta = dot(lightDir, normalize(-light.direction));

    // Do full lighting calculations for fragments inside the spotlight, otherwise only use ambient
    // QUESTION: Do we need to take the absolute value of these cosine outputs? Why does this work with negative angles?
    // Do we actually get negative angles? 
    if (theta > light.cutOff)
    {
        // Diffuse Lighting

        vec3 norm = normalize(Normal);

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

        // Calculate lighting attenuation
        float distance = length(light.position - FragPos);
        float attenuation = 1.0f / (light.constant + (light.linear * distance) + (light.quadratic * pow(distance, 2.0f)));
        lightAdjustedColor *= vec3(attenuation);

        FragColor = vec4(lightAdjustedColor, 1.0f);
    }
    else
    {
        FragColor = vec4(ambientColor, 1.0f);
    }
}