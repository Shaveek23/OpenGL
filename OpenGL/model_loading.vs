#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;
out float visibility;

const float density = 0.02;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 normal; // ta macierz jest odpowiedzialna za przekszta³cenie mapy wektorów normalnych przy obracaniu np.

void main()
{
    TexCoords = aTexCoords;    
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(normal) * aNormal;
    vec4 posRelToCam = view * vec4(FragPos, 1.0);
    gl_Position = projection * view * vec4(FragPos, 1.0);
    float distance = length(posRelToCam.xyz);
    visibility = exp(-pow(distance * density, 2));
    visibility = clamp(visibility, 0.0, 1.0);
}