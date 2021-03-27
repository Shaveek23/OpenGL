#include "SpotLight.h"

SpotLight::SpotLight(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, glm::vec3& position,
    float constant, float linear, float quadratic, const glm::vec3& direction, float cutOff, float outerCutOff)
    : PointLight(ambient, diffuse, specular, position, constant, linear, quadratic)
{
    this->direction = direction;
    this->cutOff = cutOff;
    this->outerCutOff = outerCutOff;
}

SpotLight::SpotLight() : PointLight() {};
