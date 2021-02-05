#include <scene/glm-wrapper.hpp>
#include "camera.h"

bool Camera::moving()
{
    return keys.left || keys.right || keys.up || keys.down || keys.forward || keys.back;
}

float Camera::getNearClip()
{
    return znear;
}

float Camera::getFarClip()
{
    return zfar;
}

void Camera::updateAspectRatio(float aspect)
{
    this->aspect = aspect;
}

void Camera::setPosition(glm::float3 position)
{
    mPosition = position;
}

void Camera::update(float deltaTime)
{
    updated = false;
    {
        if (moving())
        {
            const float moveSpeed = deltaTime * movementSpeed;

            if (keys.forward)
                mPosition += getFront() * moveSpeed;
            if (keys.back)
                mPosition -= getFront() * moveSpeed;
            if (keys.left)
                mPosition -= getRight() * moveSpeed;
            if (keys.right)
                mPosition += getRight() * moveSpeed;
            if (keys.up)
                mPosition += getUp() * moveSpeed;
            if (keys.down)
                mPosition -= getUp() * moveSpeed;
        }
    }
}
