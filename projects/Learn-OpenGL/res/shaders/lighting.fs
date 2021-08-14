#version 430

struct DirectionalLight
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constantFactor;
    float linearFactor;
    float quadraticFactor;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;
    float cosCutOff;
    float cosOuterCutOff;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constantFactor;
    float linearFactor;
    float quadraticFactor;
};

in vec3 Normal;
in vec3 FragmentPosition;
in vec2 TextureCoordinates;

out vec4 FragmentColor;

#define MAX_POINT_LIGHTS_COUNT 16
uniform PointLight u_pointLights[MAX_POINT_LIGHTS_COUNT];
uniform int u_pointLightsCount;

uniform DirectionalLight u_directionalLight;
uniform bool u_directionalLightEnabled;

uniform SpotLight u_spotLight;
uniform bool u_spotLightEnabled;

#define MAX_DIFFUSE_TEXTURES_COUNT 1
#define MAX_SPECULAR_TEXTURES_COUNT 1
uniform sampler2D u_diffuseTexture[MAX_DIFFUSE_TEXTURES_COUNT];
uniform sampler2D u_specularTexture[MAX_SPECULAR_TEXTURES_COUNT];
uniform float u_shininess;

vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 directionToCamera);

vec3 calculatePointLight(PointLight light,
                         vec3 normal,
                         vec3 fragmentPosition,
                         vec3 directionToCamera);

vec3 calculateSpotLight(SpotLight light,
                        vec3 normal,
                        vec3 fragmentPosition,
                        vec3 directionToCamera);

float linearizeDepth(float depth, float near, float far);

void main()
{
    vec3 normal = normalize(Normal);
    vec3 directionToCamera = normalize(-FragmentPosition);
    vec3 resultColor = vec3(0.0);

    if (u_directionalLightEnabled)
    {
        resultColor += calculateDirectionalLight(u_directionalLight, normal, directionToCamera);
    }

    for (int i = 0; i < min(MAX_POINT_LIGHTS_COUNT, u_pointLightsCount); i++)
    {
        resultColor +=
            calculatePointLight(u_pointLights[i], normal, FragmentPosition, directionToCamera);
    }

    if (u_spotLightEnabled)
    {
        resultColor += calculateSpotLight(u_spotLight, normal, FragmentPosition, directionToCamera);
    }

    FragmentColor = vec4(resultColor, 1.0);
}

vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 directionToCamera)
{
    vec3 directionToLight = normalize(-light.direction);
    vec3 reflectionDirection = reflect(-directionToLight, normal);
    float diffuseStrength = max(dot(normal, directionToLight), 0.0);
    float specularStrength =
        pow(diffuseStrength, 1.0) *
        pow(max(dot(directionToCamera, reflectionDirection), 0.0), u_shininess);

    vec3 ambient = light.ambient * vec3(texture(u_diffuseTexture[0], TextureCoordinates));
    vec3 diffuse =
        light.diffuse * diffuseStrength * vec3(texture(u_diffuseTexture[0], TextureCoordinates));
    vec3 specular =
        light.specular * specularStrength * vec3(texture(u_specularTexture[0], TextureCoordinates));

    return ambient + diffuse + specular;
}

vec3 calculatePointLight(PointLight light,
                         vec3 normal,
                         vec3 fragmentPosition,
                         vec3 directionToCamera)
{
    vec3 directionToLight = normalize(light.position - fragmentPosition);
    vec3 reflectionDirection = reflect(-directionToLight, normal);
    float diffuseStrength = max(dot(normal, directionToLight), 0.0);
    float specularStrength =
        pow(diffuseStrength, 3.0) *
        pow(max(dot(directionToCamera, reflectionDirection), 0.0), u_shininess);
    float distanceToLight = length(light.position - fragmentPosition);
    float attenuation = 1.0 / (light.constantFactor + light.linearFactor * distanceToLight +
                               light.quadraticFactor * distanceToLight * distanceToLight);

    vec3 ambient = light.ambient * vec3(texture(u_diffuseTexture[0], TextureCoordinates));
    vec3 diffuse =
        light.diffuse * diffuseStrength * vec3(texture(u_diffuseTexture[0], TextureCoordinates));
    vec3 specular =
        light.specular * specularStrength * vec3(texture(u_specularTexture[0], TextureCoordinates));

    return attenuation * (ambient + diffuse + specular);
}

vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 fragmentPosition, vec3 directionToCamera)
{
    vec3 directionToLight = normalize(light.position - fragmentPosition);
    vec3 reflectionDirection = reflect(-directionToLight, normal);
    float diffuseStrength = max(dot(normal, directionToLight), 0.0);
    float specularStrength =
        pow(diffuseStrength, 3.0) *
        pow(max(dot(directionToCamera, reflectionDirection), 0.0), u_shininess);
    float distanceToLight = length(light.position - fragmentPosition);
    float attenuation = 1.0 / (light.constantFactor + light.linearFactor * distanceToLight +
                               light.quadraticFactor * distanceToLight * distanceToLight);

    float dotLightDirection = dot(normalize(directionToLight), normalize(-light.direction));
    float intensity =
        clamp((dotLightDirection - light.cosOuterCutOff) / (light.cosCutOff - light.cosOuterCutOff),
              0.0,
              1.0);

    vec3 ambient = light.ambient * vec3(texture(u_diffuseTexture[0], TextureCoordinates));
    vec3 diffuse =
        light.diffuse * diffuseStrength * vec3(texture(u_diffuseTexture[0], TextureCoordinates));
    vec3 specular =
        light.specular * specularStrength * vec3(texture(u_specularTexture[0], TextureCoordinates));

    return intensity * attenuation * (ambient + diffuse + specular);
}

float linearizeDepth(float depth, float near, float far)
{
    // normalized device coordinates (go from range [0, 1] to [-1, 1])
    float ndc = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - ndc * (far - near));
}
