#include "Light.h"

Light::Light(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular) 
{
	this->ambient = ambient;
	this->diffuse = diffuse;
	this->specular = specular;
}

Light::Light() {}

void Light::TurnOff()
{
	_ambient = ambient;
	_diffuse = diffuse;
	_specular = specular;
	ambient = glm::vec3(0.0f);
	diffuse = glm::vec3(0.0f);
	specular = glm::vec3(0.0f);
}

void Light::TurnOn()
{
	ambient = _ambient;
	diffuse = _diffuse;
	specular = _specular;
}

