#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm defaults to opengl depth -1 to 1, Vulkan usese 0 to 1

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>

#include "Mesh.h"

class ModelLoader {
public:
    ModelLoader() = default;

    ~ModelLoader() = default;

    void cleanup();

    bool cleanup(Mesh *mesh);

    Mesh *loadMeshFromOBJ(const std::string &filename);

    Mesh *loadMeshFromPLY(const std::string &filename);

    static Mesh *getQuad();

private:
    // Meshes
    std::vector<Mesh *> m_meshes;
    // Special often used meshes
    static Mesh *m_quadMesh;
};  