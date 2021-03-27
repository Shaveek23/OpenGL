#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class AbstractCamera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Up;

    AbstractCamera(glm::vec3 position, glm::vec3 up);

    virtual glm::mat4 GetViewMatrix() = 0;

};

class Camera : public AbstractCamera
{
public:
    glm::vec3 Target;

    Camera(glm::vec3 pos, glm::vec3 target, glm::vec3 up = glm::vec3(0.0, 1.0, 0.0));

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix() override;

    void updateTarget(glm::vec3 target);
    void updatePosition(glm::vec3 position);

};

