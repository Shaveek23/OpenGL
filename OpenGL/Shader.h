#pragma once
#include <string>
#include <glm/mat4x4.hpp>
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"

class Shader
{
public:
    // the program ID
    unsigned int ID;

    // constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath);
    // use/activate the shader
    void use();
    // utility uniform functions
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec4(const std::string& name, float x, float y, float z, float w) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;

    void AddDirectionalLight(DirectionalLight* light);
    void AddPointLight(PointLight* light);
    void AddSpotLight(SpotLight* light);

    void setLightsUniforms();
  

    ~Shader();

private:
    DirectionalLight* directionalLights[10]; //changing arrays size requires changes in fragment shaderes
    SpotLight* spotLights[10];
    PointLight* pointLights[10];

    int dirCount = 0;
    int spotCount = 0;
    int pointCount = 0;

    void checkCompileErrors(unsigned int shader, std::string type);
};

