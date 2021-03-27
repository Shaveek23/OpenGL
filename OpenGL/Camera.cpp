#include "Camera.h"

Camera::Camera(glm::vec3 pos, glm::vec3 target, glm::vec3 up) : AbstractCamera(pos, up), Target(target) { }

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Target, Up);
}

void Camera::updateTarget(glm::vec3 target)
{
    Target = target;
}

void Camera::updatePosition(glm::vec3 position)
{
    Position = position;
}

AbstractCamera::AbstractCamera(glm::vec3 position, glm::vec3 up)
{
    this->Position = position;
    this->Up = up;
}
