#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm defaults to opengl depth -1 to 1, Vulkan usese 0 to 1

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <optional>
#include <memory>

#include "Mesh.h"

class ModelLoader {
public:
    ModelLoader() = default;

    ~ModelLoader() = default;

    void cleanup();

    static std::optional<std::unique_ptr<Mesh>> loadMeshFromOBJ(const std::string &filename);

    static std::optional<std::unique_ptr<Mesh>> loadMeshFromPLY(const std::string &filename);

    static Mesh getQuad();

};