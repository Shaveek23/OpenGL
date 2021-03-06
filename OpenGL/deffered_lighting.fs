#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

#define NR_LIGHTS 10


uniform vec3 viewPos;
uniform DirLight dirLights[NR_LIGHTS];
uniform PointLight pointLights[NR_LIGHTS];
uniform SpotLight spotLights[NR_LIGHTS];
uniform Material material;
uniform int dirCount;
uniform int spotCount;
uniform int pointCount;
uniform bool isFog;
uniform vec3 fogColor;


// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 Diffuse);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 Diffuse);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 Diffuse);

void main()
{   
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float visibility = texture(gAlbedoSpec, TexCoords).a;
    // properties
     vec3 norm = Normal;
     vec3 viewDir = normalize(viewPos - FragPos);
     
    // == =====================================================
    // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // == =====================================================
    vec3 result = vec3(0.0f);
    // phase 1: directional lighting
    for (int i = 0; i < dirCount; i++)
       result += CalcDirLight(dirLights[i], norm, viewDir, Diffuse);
    // phase 2: point lights
    for (int i = 0; i < pointCount; i++)
      result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, Diffuse);    
    // phase 3: spot light
    for (int i = 0; i < spotCount; i++)
       result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir, Diffuse);    

       if (isFog)
           FragColor = (visibility) * vec4(result, 1.0) + (1 - visibility) * vec4(fogColor, 1.0);
       else
           FragColor = vec4(result, 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 Diffuse)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    // combine results
    vec3 ambient = light.ambient * vec3(1.0, 1.0, 1.0);
    vec3 diffuse = light.diffuse * diff * Diffuse;
    vec3 specular = light.specular * spec * Diffuse;
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 Diffuse)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * vec3(1.0, 1.0, 1.0);
    vec3 diffuse = light.diffuse * diff * Diffuse;
    vec3 specular = light.specular * spec * Diffuse;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 Diffuse)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
     vec3 ambient = light.ambient * vec3(1.0, 1.0, 1.0);
    vec3 diffuse = light.diffuse * diff * Diffuse;
    vec3 specular = light.specular * spec * Diffuse;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}