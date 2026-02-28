#pragma once

#include "utils/glm_utils.h"
#include "utils/math_utils.h"

namespace cameraSpace
{
    class Camera
    {
    public:
        Camera(const glm::vec3& position, const glm::vec3& center, const glm::vec3& up)
        {
            Set(position, center, up);
        }

        ~Camera()
        {
        }

        // Update camera
        void Set(const glm::vec3& position, const glm::vec3& center, const glm::vec3& up)
        {
            this->position = position;
            forward = glm::normalize(center - position);
            right = glm::cross(forward, up);
            this->up = glm::cross(right, forward);
            this->distanceToTarget = glm::length(center - position);
        }

        void MoveForward(float distance)
        {
            glm::vec3 dir = glm::normalize(glm::vec3(forward.x, 0, forward.z));
            position += dir * distance;
        }

        void TranslateForward(float distance)
        {
            position += forward * distance;
        }

        void TranslateUpward(float distance)
        {
            glm::vec3 dir = glm::vec3(0, 1, 0);
            position += dir * distance;
        }

        void TranslateRight(float distance)
        {
            glm::vec3 dir = glm::normalize(glm::vec3(right.x, 0, right.z));
            position += dir * distance;
        }

        void RotateFirstPerson_OX(float angle)
        {
            glm::vec3 refDirection = right;
            glm::mat3 rotationMatrix = glm::mat3(
                glm::rotate(glm::mat4(1), angle, refDirection)
            );

            forward = rotationMatrix * forward;
            up = rotationMatrix * up;
        }

        void RotateFirstPerson_OY(float angle)
        {
            glm::vec3 refDirection = glm::vec3(0, 1, 0);
            glm::mat3 rotationMatrix = glm::mat3(
                glm::rotate(glm::mat4(1), angle, refDirection)
            );

            forward = rotationMatrix * forward;
            up = rotationMatrix * up;
            right = rotationMatrix * right;
        }

        void RotateFirstPerson_OZ(float angle)
        {
            glm::vec3 refDirection = forward;
            glm::mat3 rotationMatrix = glm::mat3(
                glm::rotate(glm::mat4(1), angle, refDirection)
            );

            right = rotationMatrix * right;
            up = rotationMatrix * up;
        }

        void RotateThirdPerson_OX(float angle)
        {
            TranslateForward(distanceToTarget);
            RotateFirstPerson_OX(angle);
            TranslateForward(-distanceToTarget);
        }

        void RotateThirdPerson_OY(float angle)
        {
            TranslateForward(distanceToTarget);
            RotateFirstPerson_OY(angle);
            TranslateForward(-distanceToTarget);
        }

        void RotateThirdPerson_OZ(float angle)
        {
            TranslateForward(distanceToTarget);
            RotateFirstPerson_OZ(angle);
            TranslateForward(-distanceToTarget);
        }

        glm::mat4 GetViewMatrix()
        {
            return glm::lookAt(position, position + forward, up);
        }

        glm::vec3 GetTargetPosition()
        {
            return position + forward * distanceToTarget;
        }

    public:
        float distanceToTarget;
        glm::vec3 position;
        glm::vec3 forward;
        glm::vec3 right;
        glm::vec3 up;
    };
}   // namespace cameraSpace
