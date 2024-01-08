#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm defaults to opengl depth -1 to 1, Vulkan is using 0 to 1

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>

#include <entt/entt.hpp>

namespace ChaosEngine::Components {
    struct Transform {
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;

        [[nodiscard]] inline glm::mat4 getModelMatrix() const {
            glm::mat4 ret = glm::translate(glm::mat4(1), position);
            ret *= glm::toMat4(
                    glm::quat({glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z)}));
            return glm::scale(ret, scale);;
        }
    };


    struct TransformComponent {
        explicit TransformComponent(const Transform &transform) : local(transform), global(transform) {}

        void setTransform(const Transform &transform) { global = transform; }

        void setPosition(const glm::vec3 &position) { global.position = position; }

        void setRotation(const glm::vec3 &rotation) { global.rotation = rotation; }

        void setScale(const glm::vec3 &scale) { global.scale = scale; }

        void setLocalTransform(const Transform &transform) { local = transform; }

        void setLocalPosition(const glm::vec3 &position) { local.position = position; }

        void setLocalRotation(const glm::vec3 &rotation) { local.rotation = rotation; }

        void setLocalScale(const glm::vec3 &scale) { local.scale = scale; }

        [[nodiscard]] const Transform &getTransform() const { return global; }

        [[nodiscard]] const Transform &getLocalTransform() const { return local; }

    private: // Internals
        Transform local;
        Transform global; // To be updated from SceneGraph system according to transform tree
    };

    struct SceneGraphComponent {
        entt::entity parent;
        entt::entity first;
        entt::entity next;
        entt::entity prev;
        bool needsUpdate;
    };
}
