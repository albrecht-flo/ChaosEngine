#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm defaults to opengl depth -1 to 1, Vulkan is using 0 to 1
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <GLFW/glfw3.h>
#include <string>

struct Transform {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    inline glm::mat4 getModelMatrix() const {
        glm::mat4 ret = glm::translate(glm::mat4(1), position);
        ret *= glm::toMat4(glm::quat({glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z)}));
        return glm::scale(ret, scale);;
    }
};

struct RenderComponent {
    glm::vec4 color;
};

struct CameraComponent {
    glm::mat4 view;
    float fieldOfView = 45.0f;
    float near = 0.1f;
    float far = 100.0f;
};