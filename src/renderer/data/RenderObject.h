#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm defaults to opengl depth -1 to 1, Vulkan usese 0 to 1

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>

#include "../vulkan/memory/VulkanBuffer.h"

// TODO: Refactor

typedef const uint32_t RenderObjectRef;
typedef const uint32_t MaterialRef;

struct RenderMesh {
    VulkanBuffer vertexBuffer;
    VulkanBuffer indexBuffer;
    uint32_t indexCount;
    uint32_t id = ids++;
public: // public needed for MSVC
    static uint32_t ids;
};

struct RenderObject {
    RenderMesh *mesh;
    glm::mat4 modelMat;
    MaterialRef material;
};

struct TexturePhongMaterial {
    const std::string textureFile;
    float shininess;
};

struct LightObject {
    glm::vec4 lightPos; // w = lightRadius
    glm::vec4 lightColor; // w = ambient ammount
};


