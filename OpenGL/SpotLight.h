#pragma once
#include "PointLight.h"
class SpotLight :
    public PointLight
{
public:
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;
    SpotLight(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, glm::vec3& position,
        float constant, float linear, float quadratic, const glm::vec3& direction, float cutOff, float outerCutOff);

    SpotLight();


};

