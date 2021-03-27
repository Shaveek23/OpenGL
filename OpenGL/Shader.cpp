#include "Shader.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tchar.h>
#include <Windows.h>
#include <GL\glew.h>
#include <glm\ext\matrix_float4x4.hpp>
#include <glm\gtc\type_ptr.hpp>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    // 2. compile shaders
    unsigned int vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::use()
{
    glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::AddDirectionalLight(DirectionalLight* light)
{
    if (dirCount < 10)
    {
        directionalLights[dirCount] = light;
        dirCount++;
    }
}

void Shader::AddPointLight(PointLight* light)
{
    if (pointCount < 10)
    {
        pointLights[pointCount] = light;
        pointCount++;
    }
}

void Shader::AddSpotLight(SpotLight* light)
{
    if (spotCount < 10)
    {
        spotLights[spotCount] = light;
        spotCount++;
    }
}

void Shader::setLightsUniforms()
{
    for (int i = 0; i < dirCount; i++)
    {
        this->setVec3("dirLights[" + std::to_string(i) + "].direction", directionalLights[i]->direction);
        this->setVec3("dirLights[" + std::to_string(i) + "].ambient", directionalLights[i]->ambient);
        this->setVec3("dirLights[" + std::to_string(i) + "].diffuse", directionalLights[i]->diffuse);
        this->setVec3("dirLights[" + std::to_string(i) + "].specular", directionalLights[i]->specular);
    }
    this->setInt("dirCount", dirCount);

    for (int i = 0; i < spotCount; i++)
    {
        this->setVec3("spotLights[" + std::to_string(i) + "].direction", spotLights[i]->direction);
        this->setVec3("spotLights[" + std::to_string(i) + "].ambient", spotLights[i]->ambient);
        this->setVec3("spotLights[" + std::to_string(i) + "].diffuse", spotLights[i]->diffuse);
        this->setVec3("spotLights[" + std::to_string(i) + "].specular", spotLights[i]->specular);
        this->setVec3("spotLights[" + std::to_string(i) + "].position", spotLights[i]->position);
        this->setFloat("spotLights[" + std::to_string(i) + "].cutOff", spotLights[i]->cutOff);
        this->setFloat("spotLights[" + std::to_string(i) + "].outerCutOff", spotLights[i]->outerCutOff);
        this->setFloat("spotLights[" + std::to_string(i) + "].quadratic", spotLights[i]->quadratic);
        this->setFloat("spotLights[" + std::to_string(i) + "].constant", spotLights[i]->constant);
        this->setFloat("spotLights[" + std::to_string(i) + "].linear", spotLights[i]->linear);
    }
    this->setInt("spotCount", spotCount);

    for (int i = 0; i < pointCount; i++)
    {
        this->setVec3("pointLights[" + std::to_string(i) + "].position", pointLights[i]->position);
        this->setVec3("pointLights[" + std::to_string(i) + "].ambient", pointLights[i]->ambient);
        this->setVec3("pointLights[" + std::to_string(i) + "].diffuse", pointLights[i]->diffuse);
        this->setVec3("pointLights[" + std::to_string(i) + "].specular", pointLights[i]->specular);
        this->setFloat("pointLights[" + std::to_string(i) + "].quadratic", pointLights[i]->quadratic);
        this->setFloat("pointLights[" + std::to_string(i) + "].constant", pointLights[i]->constant);
        this->setFloat("pointLights[" + std::to_string(i) + "].linear", pointLights[i]->linear);
    }
    this->setInt("pointCount", pointCount);


}


Shader::~Shader()
{
    glDeleteProgram(ID);
}

void Shader::checkCompileErrors(unsigned int shader, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
