#include "PointLight.h"

PointLight::PointLight(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, glm::vec3& position, float constant, float linear, float quadratic) : Light(ambient, diffuse, specular)
{
	this->constant = constant;
	this->linear = linear;
	this->quadratic = quadratic;
	this->position = position;
}

PointLight::PointLight() :Light() {}
