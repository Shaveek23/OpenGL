#include "DirectionalLight.h"

DirectionalLight::DirectionalLight(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& direction) : Light(ambient, diffuse, specular)
{
	this->direction = direction;
}

DirectionalLight::DirectionalLight(): Light() {}
