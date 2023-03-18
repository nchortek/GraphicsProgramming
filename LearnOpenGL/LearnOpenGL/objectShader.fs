#version 330 core

struct Material {
    sampler2D diffuseMap;
    sampler2D specularMap;
    float shininess;
};

struct SpotLight {
    // Orientation
    vec3 position;
    vec3 direction;

    // Color
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    // Attentuation
    float constant;
    float linear;
    float quadratic;

    float innerCutOff;
    float outerCutOff;
};

struct PointLight {
    // Orientation
    vec3 position;

    // Color
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    // Attentuation
    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight {
    // Orientation
    vec3 direction;

    // Color
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform Material material;
uniform SpotLight spotLight;
uniform DirectionalLight directionalLight;

#define NR_POINT_LIGHTS 4 
uniform PointLight pointLights[NR_POINT_LIGHTS];

uniform vec3 viewPos;

out vec4 FragColor;

vec3 ProcessDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 diffuseTexture, vec3 specularTexture);
vec3 ProcessPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 diffuseTexture, vec3 specularTexture);
vec3 ProcessSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 diffuseTexture, vec3 specularTexture);

void main()
{
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 diffuseTexture = texture(material.diffuseMap, TexCoords).rgb;
    vec3 specularTexture = texture(material.specularMap, TexCoords).rgb;

    vec3 lightAdjustedColor = ProcessDirectionalLight(directionalLight, normal, viewDir, diffuseTexture, specularTexture);
    lightAdjustedColor += ProcessSpotLight(spotLight, normal, viewDir, diffuseTexture, specularTexture);

    for (int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        lightAdjustedColor += ProcessPointLight(pointLights[i], normal, viewDir, diffuseTexture, specularTexture);
    }

    FragColor = vec4(lightAdjustedColor, 1.0);
}

vec3 ProcessDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 diffuseTexture, vec3 specularTexture)
{
    // Ambient
    vec3 ambientColor = light.ambient * diffuseTexture;

    // Diffuse
    vec3 lightDir = normalize(-light.direction);
    float diffuseFactor = max(dot(normal, lightDir), 0.0f);
    vec3 diffuseColor = light.diffuse * diffuseFactor * diffuseTexture;

    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
    vec3 specularColor = light.specular * specularFactor * specularTexture;

    vec3 lightColor = ambientColor + diffuseColor + specularColor;
    return lightColor;
}

vec3 ProcessPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 diffuseTexture, vec3 specularTexture)
{
    // Ambient
    vec3 ambientColor = light.ambient * diffuseTexture;

    // Diffuse
    vec3 lightDir = normalize(light.position - FragPos);
    float diffuseFactor = max(dot(normal, lightDir), 0.0f);
    vec3 diffuseColor = light.diffuse * diffuseFactor * diffuseTexture;

    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
    vec3 specularColor = light.specular * specularFactor * specularTexture;

    // Attenuation
    float distance = length(light.position - FragPos);
    float attenuation = 1.0f / (light.constant + (light.linear * distance) + (light.quadratic * distance * distance));

    vec3 lightColor = (ambientColor + diffuseColor + specularColor);
    return (lightColor * attenuation);
}

vec3 ProcessSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 diffuseTexture, vec3 specularTexture)
{
    // We use ambient lighting for objects both inside and outside the spotlight so calculate it first
    vec3 ambientColor = light.ambient * diffuseTexture;

    vec3 lightDir = normalize(light.position - FragPos);
    float theta = dot(lightDir, normalize(-light.direction));

    // Do full lighting calculations for fragments inside the spotlight, otherwise only use ambient
    // QUESTION: Do we need to take the absolute value of these cosine outputs? Why does this work with negative angles?
    // Do we actually get negative angles? 
    if (theta > light.outerCutOff)
    {
        // Diffuse

        // Make sure to clamp lower values to 0 (dot products become negative for angles greater than 90 degrees)
        float diffuseFactor = max(dot(normal, lightDir), 0.0f);
        vec3 diffuseColor = light.diffuse * diffuseFactor * diffuseTexture;

        // Specular

        // The reflect function expects the first vector to point from the light source towards the fragment's position,
        // but the lightDir vector is currently pointing the other way around: from the fragment towards the light source
        // (this depends on the order of subtraction earlier on when we calculated the lightDir vector). So we just negate
        // lightDir here.
        vec3 reflectDir = reflect(-lightDir, normal);
        float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
        vec3 specularColor = light.specular * specularFactor * specularTexture;

        // We'll add ambient later after adjusting for attenuation and spotlight intensity because ambient
        // should be unaffected by our lightsource
        vec3 lightAdjustedColor = diffuseColor + specularColor;

        // Attenuation
        float distance = length(light.position - FragPos);
        float attenuation = 1.0f / (light.constant + (light.linear * distance) + (light.quadratic * distance * distance));
        lightAdjustedColor *= attenuation;

        // Spotlight fade/intensity
        float spotlightIntensity = (theta - light.outerCutOff) / (light.innerCutOff - light.outerCutOff);
        spotlightIntensity = clamp(spotlightIntensity, 0.0f, 1.0f);
        lightAdjustedColor *= spotlightIntensity;

        lightAdjustedColor += ambientColor;
        return lightAdjustedColor;
    }

    // TODO: Do we need to apply attentuation to this? Or return ambient color at all if the fragment is outside the cone?
    return ambientColor;
}