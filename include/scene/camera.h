#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
private:
    glm::float3 mPosition = { 1.0f, 0.0f, 0.0f };
    glm::quat mOrientation = { 1.0f, 0.0f, 0.0f, 0.0f };

    glm::float3 getFront()
    {
        return glm::conjugate(mOrientation) * glm::float3(0, 0, -1);
    }

    glm::float3 getUp()
    {
        return glm::conjugate(mOrientation) * glm::float3(0, 1, 0);
    }

    glm::float3 getRight()
    {
        return glm::conjugate(mOrientation) * glm::float3(1, 0, 0);
    }

    float fov = 45.0f;
    float aspect = 1.0f;
    float znear = 0.01f, zfar = 10000.0f;

public:
    glm::float4x4 getView()
    {
        return glm::mat4_cast(mOrientation) * glm::translate(glm::float4x4(1.0f), -mPosition);
    }

    glm::float4x4 getPerspective()
    {
        return glm::perspective(fov, aspect, znear, zfar);
    }

    void onMouseMove(const float dx, const float dy)
    {
        glm::quat pitch = glm::angleAxis(dy * rotationSpeed, glm::float3(1, 0, 0));
        glm::quat yaw = glm::angleAxis(dx * rotationSpeed, glm::float3(0, 1, 0));

        mOrientation = glm::normalize(pitch * mOrientation * yaw);
    }

    float rotationSpeed;
    float movementSpeed;

    bool updated = false;

    struct MouseButtons
    {
        bool left = false;
        bool right = false;
        bool middle = false;
    } mouseButtons;

    glm::float2 mousePos = { 0.0f, 0.0f };

    struct
    {
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
        bool forward = false;
        bool back = false;
    } keys;

    bool moving();
    float getNearClip();
    float getFarClip();
    void updateAspectRatio(float aspect);
    void setFov(float fov)
    {
        this->fov = fov;
    }
    void setPosition(glm::float3 position);
    void update(float deltaTime);
};
